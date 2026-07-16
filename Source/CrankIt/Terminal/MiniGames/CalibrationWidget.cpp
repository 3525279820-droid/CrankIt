// Fill out your copyright notice in the Description page of Project Settings.


#include "CalibrationWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"
#include "Components/PanelWidget.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

TSharedRef<SWidget> UCalibrationWidget::RebuildWidget()
{
	// BuildRuntimeTextUI 在此处调用，确保 WidgetTree->RootWidget 在
	// Super::RebuildWidget() 构建 Slate SObjectWidget 内容之前已经就绪。
	BuildRuntimeTextUI();
	return Super::RebuildWidget();
}

void UCalibrationWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildRuntimeTextUI();
	StartGame();
}

void UCalibrationWidget::NativeDestruct()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(MoveTimerHandle);
	}
	Super::NativeDestruct();
}

void UCalibrationWidget::StartGame()
{
	GridSize = FMath::Max(1, GridSize);
	MinMoveInterval = FMath::Max(0.01f, MinMoveInterval);
	MoveInterval = FMath::Max(MinMoveInterval, MoveInterval);

	if (CellBorders.Num() == 0)
	{
		BuildRuntimeTextUI();
	}

	// 逻辑行从底部开始递增（row 1 -> row 8），每行记录已锁定列。
	LockedColsPerRow.Init(INDEX_NONE, GridSize);
	CurrentRow = 0;
	CurrentMovingCol = 0;
	LockedColumn = INDEX_NONE;
	bGameActive = true;
	ActiveMoveInterval = MoveInterval;

	RestartMoveTimer();
	RefreshRuntimeTextUI();
}

void UCalibrationWidget::RestartMoveTimer()
{
	if (!GetWorld() || !bGameActive)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(MoveTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		MoveTimerHandle,
		this,
		&UCalibrationWidget::MoveCurrentBlockRight,
		FMath::Max(MinMoveInterval, ActiveMoveInterval),
		true
	);
}

void UCalibrationWidget::ResetGame()
{
	StartGame();
}

void UCalibrationWidget::HandleKey(const FKey& Key)
{
	if (!bGameActive || Key != EKeys::Enter)
	{
		return;
	}
	UE_LOG(LogTemp, Display, TEXT("Enter pressed."))

	// 首次锁定可任意列；后续必须与首次列一致。
	const int32 CurrentCol = CurrentMovingCol;
	if (LockedColumn == INDEX_NONE)
	{
		LockedColumn = CurrentCol;
	}
	else if (CurrentCol != LockedColumn)
	{
		// 列不一致则整局重置
		ResetGame();
		return;
	}

	if (LockedColsPerRow.IsValidIndex(CurrentRow))
	{
		LockedColsPerRow[CurrentRow] = CurrentCol;
	}

	if (CurrentRow >= GridSize - 1)
	{
		bGameActive = false;
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(MoveTimerHandle);
		}
		RefreshRuntimeTextUI();
		OnGameFinished.Broadcast(true);
		return;
	}

	// 锁定成功后进入上一逻辑行，并从最左列重新开始移动；略微加快移动速度。
	ActiveMoveInterval = FMath::Max(MinMoveInterval, ActiveMoveInterval - MoveFasterPerRow);
	CurrentRow++;
	CurrentMovingCol = 0;
	RestartMoveTimer();
	RefreshRuntimeTextUI();
}

void UCalibrationWidget::BuildRuntimeTextUI()
{
	if (!WidgetTree)
	{
		return;
	}

	if (CellBorders.Num() > 0)
	{
		return;
	}

	GridSize = FMath::Max(1, GridSize);
	CellBorders.Empty();
	CellBorders.SetNumZeroed(GridSize * GridSize);
	RowLabelTexts.Empty();
	RowLabelTexts.SetNumZeroed(GridSize);
	ColLabelTexts.Empty();
	ColLabelTexts.SetNumZeroed(GridSize);

	UVerticalBox* RuntimeRoot = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CalibrationRoot"));
	if (WidgetTree->RootWidget == nullptr)
	{
		WidgetTree->RootWidget = RuntimeRoot;
	}
	else if (UPanelWidget* ExistingRoot = Cast<UPanelWidget>(WidgetTree->RootWidget))
	{
		// 若蓝图已有 Panel 根节点，则把运行时 UI 挂在其下方。
		ExistingRoot->AddChild(RuntimeRoot);
	}
	else
	{
		return;
	}

	UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CalibrationTitle"));
	TitleText->SetText(FText::FromString(TEXT("Calibration Tool\nline up fuel rod in a straight line"
										   "press enter to place")));
	TitleText->SetColorAndOpacity(MainColor);
	ApplyConfiguredFont(TitleText);
	RuntimeRoot->AddChildToVerticalBox(TitleText);

	StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CalibrationStatus"));
	StatusText->SetColorAndOpacity(MainColor);
	ApplyConfiguredFont(StatusText);
	if (UVerticalBoxSlot* StatusSlot = RuntimeRoot->AddChildToVerticalBox(StatusText))
	{
		StatusSlot->SetPadding(FMargin(0.f, 4.f, 0.f, 8.f));
	}

	// 单一 GridPanel：第 0 列为行号，第 1..GridSize 列为棋盘格；最后一行为列字母，与上方列严格对齐。
	BoardGrid = WidgetTree->ConstructWidget<UGridPanel>(UGridPanel::StaticClass(), TEXT("CalibrationGrid"));
	if (UVerticalBoxSlot* BoardSlot = RuntimeRoot->AddChildToVerticalBox(BoardGrid))
	{
		BoardSlot->SetPadding(FMargin(0.f, 4.f, 0.f, 0.f));
	}

	BoardGrid->SetColumnFill(0, 0.f);
	for (int32 c = 1; c <= GridSize; ++c)
	{
		BoardGrid->SetColumnFill(c, 1.f);
	}
	for (int32 r = 0; r < GridSize; ++r)
	{
		BoardGrid->SetRowFill(r, 1.f);
	}
	BoardGrid->SetRowFill(GridSize, 0.f);

	for (int32 DisplayRow = 0; DisplayRow < GridSize; ++DisplayRow)
	{
		const int32 LogicalRow = GridSize - 1 - DisplayRow;

		UTextBlock* RowLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		RowLabel->SetText(FText::FromString(FString::FromInt(LogicalRow + 1)));
		RowLabel->SetColorAndOpacity(MainColor);
		ApplyConfiguredFont(RowLabel);
		if (UGridSlot* RowSlot = BoardGrid->AddChildToGrid(RowLabel, DisplayRow, 0))
		{
			RowSlot->SetHorizontalAlignment(HAlign_Right);
			RowSlot->SetVerticalAlignment(VAlign_Center);
			RowSlot->SetPadding(FMargin(0.f, 2.f, 8.f, 2.f));
		}
		RowLabelTexts[DisplayRow] = RowLabel;

		for (int32 Col = 0; Col < GridSize; ++Col)
		{
			const int32 Index = ToCellIndex(LogicalRow, Col);
			UBorder* Cell = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
			Cell->SetPadding(FMargin(10.f));
			Cell->SetBrushColor(FLinearColor(MainColor.R, MainColor.G, MainColor.B, 0.12f));
			if (UGridSlot* CellSlot = BoardGrid->AddChildToGrid(Cell, DisplayRow, Col + 1))
			{
				CellSlot->SetPadding(FMargin(2.f));
				CellSlot->SetHorizontalAlignment(HAlign_Fill);
				CellSlot->SetVerticalAlignment(VAlign_Fill);
			}
			CellBorders[Index] = Cell;
		}
	}

	USpacer* CornerSpacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), TEXT("CalibrationCornerSpacer"));
	CornerSpacer->SetSize(FVector2D(4.f, 4.f));
	if (UGridSlot* CornerSlot = BoardGrid->AddChildToGrid(CornerSpacer, GridSize, 0))
	{
		CornerSlot->SetHorizontalAlignment(HAlign_Left);
		CornerSlot->SetVerticalAlignment(VAlign_Center);
	}

	for (int32 Col = 0; Col < GridSize; ++Col)
	{
		UTextBlock* ColLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		ColLabel->SetText(FText::FromString(FString::Chr(TEXT('A') + Col)));
		ColLabel->SetColorAndOpacity(MainColor);
		ApplyConfiguredFont(ColLabel);
		if (UGridSlot* ColSlot = BoardGrid->AddChildToGrid(ColLabel, GridSize, Col + 1))
		{
			ColSlot->SetHorizontalAlignment(HAlign_Center);
			ColSlot->SetVerticalAlignment(VAlign_Center);
			ColSlot->SetPadding(FMargin(2.f, 6.f, 2.f, 2.f));
		}
		ColLabelTexts[Col] = ColLabel;
	}
}

void UCalibrationWidget::ApplyConfiguredFont(UTextBlock* TextBlock) const
{
	if (TextBlock && Font.HasValidFont())
	{
		TextBlock->SetFont(Font);
	}
}

void UCalibrationWidget::RefreshRuntimeTextUI()
{
	const FLinearColor EmptyColor(MainColor.R, MainColor.G, MainColor.B, 0.12f);
	const FLinearColor LockedColor(MainColor.R, MainColor.G, MainColor.B, 0.95f);
	const FLinearColor MovingColor(MainColor.R, MainColor.G, MainColor.B, 0.95f);

	for (int32 LogicalRow = 0; LogicalRow < GridSize; ++LogicalRow)
	{
		for (int32 Col = 0; Col < GridSize; ++Col)
		{
			const int32 Index = ToCellIndex(LogicalRow, Col);
			if (!CellBorders.IsValidIndex(Index) || !CellBorders[Index])
			{
				continue;
			}

			FLinearColor NewColor = EmptyColor;
			const bool bLocked = LockedColsPerRow.IsValidIndex(LogicalRow) && LockedColsPerRow[LogicalRow] == Col;
			const bool bMoving = bGameActive && LogicalRow == CurrentRow && Col == CurrentMovingCol;

			// 优先显示锁定态，其次才是当前移动态，避免视觉覆盖冲突。
			if (bLocked)
			{
				NewColor = LockedColor;
			}
			else if (bMoving)
			{
				NewColor = MovingColor;
			}
			CellBorders[Index]->SetBrushColor(NewColor);
		}
	}

	// 状态测试代码，取消注释显示状态
	
	// if (StatusText)
	// {
	// 	if (!bGameActive && LockedColsPerRow.Num() == GridSize && LockedColsPerRow.Last() != INDEX_NONE)
	// 	{
	// 		StatusText->SetText(FText::FromString(TEXT("Calibration Complete")));
	// 	}
	// 	else if (LockedColumn == INDEX_NONE)
	// 	{
	// 		StatusText->SetText(FText::FromString(TEXT("Press Enter to lock row 1")));
	// 	}
	// 	else
	// 	{
	// 		const FString ColName = FString::Chr(TEXT('A') + LockedColumn);
	// 		StatusText->SetText(FText::FromString(FString::Printf(TEXT("Locked column: %s  |  Next row: %d"), *ColName, CurrentRow + 1)));
	// 	}
	// }
}

void UCalibrationWidget::MoveCurrentBlockRight()
{
	if (!bGameActive)
	{
		return;
	}

	// 循环移动：到达最右后回到 A 列。
	CurrentMovingCol = (CurrentMovingCol + 1) % GridSize;
	RefreshRuntimeTextUI();
}

int32 UCalibrationWidget::ToCellIndex(int32 LogicalRow, int32 Col) const
{
	return LogicalRow * GridSize + Col;
}


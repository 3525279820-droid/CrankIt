// Fill out your copyright notice in the Description page of Project Settings.


#include "UClassificationGameWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void UClassificationGameWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildRuntimeTextUI();
	RefreshRuntimeTextUI();
}

void UClassificationGameWidget::StartGame(const TArray<FClassificationItem>& InItems)
{
	Items = InItems;

	const int32 Total = FMath::Max(0, GridRows * GridCols);
	if (Items.Num() < Total)
	{
		// 不足时补齐（用重复项占位，方便先把流程跑通）
		const int32 Missing = Total - Items.Num();
		for (int32 i = 0; i < Missing; ++i)
		{
			FClassificationItem Dummy;
			Dummy.Id = FName(*FString::Printf(TEXT("Dummy_%d"), i));
			Dummy.CorrectCategory = EClassificationCategory::Animal;
			Items.Add(Dummy);
		}
	}
	else if (Items.Num() > Total)
	{
		Items.SetNum(Total);
	}

	bClassifiedCorrectly.Init(false, Total);
	ImageCursorIndex = 0;
	OptionCursor = EClassificationCategory::Animal;

	RefreshRuntimeTextUI();
	BP_OnStateChanged(ImageCursorIndex, OptionCursor);
}

void UClassificationGameWidget::HandleKey(const FKey& Key)
{
	if (Key == EKeys::A || Key == EKeys::Left)
	{
		MoveOptionLeft();
		RefreshRuntimeTextUI();
		BP_OnStateChanged(ImageCursorIndex, OptionCursor);
		return;
	}

	if (Key == EKeys::D || Key == EKeys::Right)
	{
		MoveOptionRight();
		RefreshRuntimeTextUI();
		BP_OnStateChanged(ImageCursorIndex, OptionCursor);
		return;
	}

	if (Key == EKeys::Enter)
	{
		ConfirmChoice();
		RefreshRuntimeTextUI();
		BP_OnStateChanged(ImageCursorIndex, OptionCursor);
		return;
	}
}

void UClassificationGameWidget::MoveOptionLeft()
{
	UE_LOG(LogTemp, Display, TEXT("left"))
	switch (OptionCursor)
	{
	case EClassificationCategory::Animal:
		OptionCursor = EClassificationCategory::Sport;
		break;
	case EClassificationCategory::Fruit:
		OptionCursor = EClassificationCategory::Animal;
		break;
	case EClassificationCategory::Sport:
		OptionCursor = EClassificationCategory::Fruit;
		break;
	}
}

void UClassificationGameWidget::MoveOptionRight()
{
	UE_LOG(LogTemp, Display, TEXT("right"))
	switch (OptionCursor)
	{
	case EClassificationCategory::Animal:
		OptionCursor = EClassificationCategory::Fruit;
		break;
	case EClassificationCategory::Fruit:
		OptionCursor = EClassificationCategory::Sport;
		break;
	case EClassificationCategory::Sport:
		OptionCursor = EClassificationCategory::Animal;
		break;
	}
}

void UClassificationGameWidget::ConfirmChoice()
{
	const int32 Total = GridRows * GridCols;
	if (Total <= 0 || Items.Num() < Total || !bClassifiedCorrectly.IsValidIndex(ImageCursorIndex))
	{
		return;
	}

	const EClassificationCategory Chosen = OptionCursor;
	const EClassificationCategory Correct = Items[ImageCursorIndex].CorrectCategory;

	if (Chosen == Correct)
	{
		bClassifiedCorrectly[ImageCursorIndex] = true;
		BP_OnClassificationCorrect(ImageCursorIndex, Chosen);

		// 找到下一个未完成的图片
		int32 NextIndex = INDEX_NONE;
		for (int32 i = 0; i < bClassifiedCorrectly.Num(); ++i)
		{
			if (!bClassifiedCorrectly[i])
			{
				NextIndex = i;
				break;
			}
		}

		if (NextIndex == INDEX_NONE)
		{
			RefreshRuntimeTextUI();
			OnGameFinished.Broadcast(true);
			return;
		}

		ImageCursorIndex = NextIndex;
		return;
	}

	BP_OnClassificationWrong(ImageCursorIndex, Chosen);
	// 这里不推进图片光标，等待玩家重新选择
}

bool UClassificationGameWidget::IsAllCorrect() const
{
	for (const bool bOk : bClassifiedCorrectly)
	{
		if (!bOk)
		{
			return false;
		}
	}
	return true;
}

void UClassificationGameWidget::BuildRuntimeTextUI()
{
	if (!WidgetTree)
	{
		return;
	}

	// 已构建过则不重复构建
	if (CellTextBlocks.Num() > 0 && OptionTextBlocks.Num() > 0)
	{
		return;
	}

	const int32 Total = FMath::Max(0, GridRows * GridCols);
	CellTextBlocks.Empty();
	CellTextBlocks.SetNumZeroed(Total);
	OptionTextBlocks.Empty();
	OptionTextBlocks.SetNumZeroed(3);

	UVerticalBox* RuntimeRoot = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RuntimeRoot"));

	// 如果蓝图已经有 RootWidget，则尝试把 RuntimeRoot 挂到现有 Root 下；
	// 否则直接把 RuntimeRoot 设为根。
	if (WidgetTree->RootWidget == nullptr)
	{
		WidgetTree->RootWidget = RuntimeRoot;
	}
	else
	{
		if (UPanelWidget* ExistingRootPanel = Cast<UPanelWidget>(WidgetTree->RootWidget))
		{
			ExistingRootPanel->AddChild(RuntimeRoot);
		}
		else
		{
			// Root 不是 Panel，无法附加子控件，直接放弃
			return;
		}
	}

	UGridPanel* Grid = WidgetTree->ConstructWidget<UGridPanel>(UGridPanel::StaticClass(), TEXT("CellGrid"));
	if (UVerticalBoxSlot* GridSlot = RuntimeRoot->AddChildToVerticalBox(Grid))
	{
		GridSlot->SetPadding(FMargin(8.f));
	}

	for (int32 Row = 0; Row < GridRows; ++Row)
	{
		for (int32 Col = 0; Col < GridCols; ++Col)
		{
			const int32 Index = Row * GridCols + Col;
			UBorder* CellBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
			CellBorder->SetPadding(FMargin(6.f));

			UTextBlock* CellText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
			CellText->SetText(FText::FromString(TEXT("Item")));
			CellText->SetColorAndOpacity(FLinearColor(1.f, 0.f, 0.f, 1.f));
			CellBorder->SetContent(CellText);

			if (UGridSlot* CellSlot = Grid->AddChildToGrid(CellBorder, Row, Col))
			{
				CellSlot->SetPadding(FMargin(2.f));
			}

			CellTextBlocks[Index] = CellText;
		}
	}

	UHorizontalBox* Options = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("OptionsRow"));
	if (UVerticalBoxSlot* OptionSlot = RuntimeRoot->AddChildToVerticalBox(Options))
	{
		OptionSlot->SetPadding(FMargin(8.f, 14.f, 8.f, 8.f));
	}

	static const TCHAR* OptionNames[3] = { TEXT("animal"), TEXT("fruit"), TEXT("sport") };
	for (int32 i = 0; i < 3; ++i)
	{
		UTextBlock* OptionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		OptionText->SetText(FText::FromString(OptionNames[i]));
		if (UHorizontalBoxSlot* HSlot = Options->AddChildToHorizontalBox(OptionText))
		{
			HSlot->SetPadding(FMargin(12.f, 0.f));
		}
		OptionTextBlocks[i] = OptionText;
	}
}

void UClassificationGameWidget::RefreshRuntimeTextUI()
{
	for (int32 i = 0; i < CellTextBlocks.Num(); ++i)
	{
		if (!CellTextBlocks[i])
		{
			continue;
		}

		const FString ItemName = Items.IsValidIndex(i) ? Items[i].Id.ToString() : FString::Printf(TEXT("Item_%02d"), i + 1);
		const bool bCursor = (i == ImageCursorIndex);
		const bool bDone = bClassifiedCorrectly.IsValidIndex(i) ? bClassifiedCorrectly[i] : false;

		FString Prefix = TEXT("[ ] ");
		if (bDone)
		{
			Prefix = TEXT("[OK] ");
		}
		if (bCursor)
		{
			Prefix = TEXT("[>] ");
		}
		CellTextBlocks[i]->SetText(FText::FromString(Prefix + ItemName));
	}

	const EClassificationCategory OptionList[3] = {
		EClassificationCategory::Animal,
		EClassificationCategory::Fruit,
		EClassificationCategory::Sport
	};
	for (int32 i = 0; i < OptionTextBlocks.Num(); ++i)
	{
		if (!OptionTextBlocks[i])
		{
			continue;
		}
		const bool bSelected = OptionList[i] == OptionCursor;
		const FString Text = FString::Printf(TEXT("%s%s"), bSelected ? TEXT("> ") : TEXT("  "), *CategoryToString(OptionList[i]));
		OptionTextBlocks[i]->SetText(FText::FromString(Text));
	}
}

FString UClassificationGameWidget::CategoryToString(EClassificationCategory Category) const
{
	switch (Category)
	{
	case EClassificationCategory::Animal:
		return TEXT("animal");
	case EClassificationCategory::Fruit:
		return TEXT("fruit");
	case EClassificationCategory::Sport:
		return TEXT("sport");
	default:
		return TEXT("unknown");
	}
}


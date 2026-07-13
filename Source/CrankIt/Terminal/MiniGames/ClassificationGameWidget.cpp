#include "ClassificationGameWidget.h"

#include "ClassificationImageLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"

void UClassificationGameWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildRuntimeImageUI();
	RefreshRuntimeImageUI();
}

// 经 FClassificationImageLibrary 扫描 ImageContentPath 并随机填充 Items
bool UClassificationGameWidget::LoadRandomItemsFromContentFolder()
{
	TArray<FClassificationImageLibrary::FSourceImage> Pool;
	if (!FClassificationImageLibrary::LoadImagesFromContentPath(ImageContentPath, Pool))
	{
		return false;
	}

	const int32 Total = FMath::Max(0, GridRows * GridCols);
	return FClassificationImageLibrary::BuildRandomRound(Pool, Total, Items);
}

// 加载随机图片并重置光标；失败时广播 OnGameFinished(false)
void UClassificationGameWidget::StartGame()
{
	if (!LoadRandomItemsFromContentFolder())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("ClassificationGameWidget: Failed to load images from '%s'."),
			*ImageContentPath);
		OnGameFinished.Broadcast(false);
		return;
	}

	const int32 Total = Items.Num();
	bClassifiedCorrectly.Init(false, Total);
	ImageCursorIndex = 0;
	OptionCursor = EClassificationCategory::Animal;

	RefreshRuntimeImageUI();
	BP_OnStateChanged(ImageCursorIndex, OptionCursor);
}

// A/D 切换底部类别，Enter 确认当前格子的分类
void UClassificationGameWidget::HandleKey(const FKey& Key)
{
	if (Key == EKeys::A || Key == EKeys::Left)
	{
		MoveOptionLeft();
		RefreshRuntimeImageUI();
		BP_OnStateChanged(ImageCursorIndex, OptionCursor);
		return;
	}

	if (Key == EKeys::D || Key == EKeys::Right)
	{
		MoveOptionRight();
		RefreshRuntimeImageUI();
		BP_OnStateChanged(ImageCursorIndex, OptionCursor);
		return;
	}

	if (Key == EKeys::Enter)
	{
		ConfirmChoice();
		RefreshRuntimeImageUI();
		BP_OnStateChanged(ImageCursorIndex, OptionCursor);
	}
}

void UClassificationGameWidget::MoveOptionLeft()
{
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

// 答对则标记并跳到下一未完成格；全部完成则 OnGameFinished(true)
void UClassificationGameWidget::ConfirmChoice()
{
	const int32 Total = Items.Num();
	if (Total <= 0 || !bClassifiedCorrectly.IsValidIndex(ImageCursorIndex))
	{
		return;
	}

	const EClassificationCategory Chosen = OptionCursor;
	const EClassificationCategory Correct = Items[ImageCursorIndex].CorrectCategory;

	if (Chosen == Correct)
	{
		bClassifiedCorrectly[ImageCursorIndex] = true;
		BP_OnClassificationCorrect(ImageCursorIndex, Chosen);

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
			RefreshRuntimeImageUI();
			OnGameFinished.Broadcast(true);
			return;
		}

		ImageCursorIndex = NextIndex;
		return;
	}

	BP_OnClassificationWrong(ImageCursorIndex, Chosen);
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

// 将纹理设为等大 Brush（边长 CellImageSize）
void UClassificationGameWidget::ApplyCellImageBrush(UImage* ImageWidget, UTexture2D* Texture) const
{
	if (!ImageWidget)
	{
		return;
	}

	FSlateBrush Brush;
	if (Texture)
	{
		Brush.SetResourceObject(Texture);
	}
	Brush.ImageSize = FVector2D(CellImageSize, CellImageSize);
	Brush.DrawAs = ESlateBrushDrawType::Image;
	ImageWidget->SetBrush(Brush);
}

// 光标格黄色半透明底，已完成格绿色半透明底
void UClassificationGameWidget::ApplyCellBorderState(UBorder* Border, bool bCursor, bool bDone) const
{
	if (!Border)
	{
		return;
	}

	FLinearColor Tint = FLinearColor(0.f, 0.f, 0.f, 0.f);
	if (bDone)
	{
		Tint = FLinearColor(0.f, 0.45f, 0.f, 0.35f);
	}
	if (bCursor)
	{
		Tint = FLinearColor(1.f, 0.85f, 0.f, 0.55f);
	}
	Border->SetBrushColor(Tint);
}

// 纯 C++ 构建 GridPanel + 底部三档选项 TextBlock（蓝图无布局时也能跑通）
void UClassificationGameWidget::BuildRuntimeImageUI()
{
	if (!WidgetTree)
	{
		return;
	}

	const int32 Total = FMath::Max(0, GridRows * GridCols);
	if (CellImages.Num() > 0 && OptionTextBlocks.Num() > 0)
	{
		return;
	}

	CellBorders.Empty();
	CellBorders.SetNumZeroed(Total);
	CellImages.Empty();
	CellImages.SetNumZeroed(Total);
	OptionTextBlocks.Empty();
	OptionTextBlocks.SetNumZeroed(3);

	UVerticalBox* RuntimeRoot = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RuntimeRoot"));

	if (WidgetTree->RootWidget == nullptr)
	{
		WidgetTree->RootWidget = RuntimeRoot;
	}
	else if (UPanelWidget* ExistingRootPanel = Cast<UPanelWidget>(WidgetTree->RootWidget))
	{
		ExistingRootPanel->AddChild(RuntimeRoot);
	}
	else
	{
		return;
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
			CellBorder->SetPadding(FMargin(2.f));

			USizeBox* CellSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
			CellSize->SetWidthOverride(CellImageSize);
			CellSize->SetHeightOverride(CellImageSize);

			UImage* CellImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
			ApplyCellImageBrush(CellImage, nullptr);
			CellSize->AddChild(CellImage);
			CellBorder->SetContent(CellSize);

			if (UGridSlot* CellSlot = Grid->AddChildToGrid(CellBorder, Row, Col))
			{
				CellSlot->SetPadding(FMargin(2.f));
			}

			CellBorders[Index] = CellBorder;
			CellImages[Index] = CellImage;
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
		ApplyConfiguredFont(OptionText);
		if (UHorizontalBoxSlot* HSlot = Options->AddChildToHorizontalBox(OptionText))
		{
			HSlot->SetPadding(FMargin(12.f, 0.f));
		}
		OptionTextBlocks[i] = OptionText;
	}
}

void UClassificationGameWidget::ApplyConfiguredFont(UTextBlock* TextBlock) const
{
	if (TextBlock && Font.HasValidFont())
	{
		TextBlock->SetFont(Font);
	}
}

// 刷新网格纹理与边框高亮、底部选项选中态
void UClassificationGameWidget::RefreshRuntimeImageUI()
{
	for (int32 i = 0; i < CellImages.Num(); ++i)
	{
		UTexture2D* Texture = Items.IsValidIndex(i) ? Items[i].Image : nullptr;
		if (CellImages[i])
		{
			ApplyCellImageBrush(CellImages[i], Texture);
		}

		const bool bCursor = (i == ImageCursorIndex);
		const bool bDone = bClassifiedCorrectly.IsValidIndex(i) ? bClassifiedCorrectly[i] : false;
		if (CellBorders.IsValidIndex(i))
		{
			ApplyCellBorderState(CellBorders[i], bCursor, bDone);
		}
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

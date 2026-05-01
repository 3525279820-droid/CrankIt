#include "SubtitleWidget.h"

#include "SubtitleSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Styling/CoreStyle.h"

void USubtitleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 无 UMG 子类时（直接 CreateWidget<USubtitleWidget>）没有设计器树，BindWidgetOptional 为空，这里补一条底栏字幕。
	if (!SubtitleText && WidgetTree)
	{
		UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("SubtitleRoot"));
		RootOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		SubtitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SubtitleText"));
		SubtitleText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 22));
		SubtitleText->SetColorAndOpacity(FLinearColor::White);
		SubtitleText->SetShadowOffset(FVector2D(1.f, 1.f));
		SubtitleText->SetShadowColorAndOpacity(FLinearColor::Black);
		SubtitleText->SetJustification(ETextJustify::Center);
		SubtitleText->SetAutoWrapText(true);
		if (UOverlaySlot* OverlaySlot = RootOverlay->AddChildToOverlay(SubtitleText))
		{
			OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
			OverlaySlot->SetVerticalAlignment(VAlign_Bottom);
			OverlaySlot->SetPadding(FMargin(48.f, 0.f, 48.f, 72.f));
		}
		WidgetTree->RootWidget = RootOverlay;
		RebuildWidget();
	}

	// RebuildWidget / 蓝图设计器根节点可能仍为 Visible，会挡第一次世界点击；在整棵树就绪后再统一关掉命中与焦点。
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	SetIsFocusable(false);
	if (UWidget* RW = GetRootWidget())
	{
		RW->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	// World 子系统随关卡存在；Widget 进树后再绑定，保证 GetWorld() 有效。
	if (UWorld* World = GetWorld())
	{
		if (USubtitleSubsystem* Subsys = World->GetSubsystem<USubtitleSubsystem>())
		{
			Subsys->OnSubtitleLineChanged.AddDynamic(this, &USubtitleWidget::HandleSubtitleLineChanged);
		}
	}
}

void USubtitleWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		if (USubtitleSubsystem* Subsys = World->GetSubsystem<USubtitleSubsystem>())
		{
			Subsys->OnSubtitleLineChanged.RemoveDynamic(this, &USubtitleWidget::HandleSubtitleLineChanged);
		}
	}
	Super::NativeDestruct();
}

void USubtitleWidget::HandleSubtitleLineChanged(FText InSubtitleLine)
{
	if (!SubtitleText)
	{
		return;
	}
	SubtitleText->SetText(InSubtitleLine);
	// 无字时 Collapsed，避免占位块挡点击；有字时 HitTestInvisible，不拦截下层全屏点击（可按项目改）。
	const bool bShow = !InSubtitleLine.IsEmpty();
	SubtitleText->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

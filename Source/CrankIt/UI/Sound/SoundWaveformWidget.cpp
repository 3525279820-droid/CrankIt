// Fill out your copyright notice in the Description page of Project Settings.

#include "SoundWaveformWidget.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void USoundWaveformWidget::ApplySegmentStyle(UWidget* SegmentWidget, bool bLit, const FLinearColor& Filled, const FLinearColor& Empty)
{
	if (!IsValid(SegmentWidget))
	{
		return;
	}

	const FLinearColor Color = bLit ? Filled : Empty;

	if (UImage* const Img = Cast<UImage>(SegmentWidget))
	{
		Img->SetColorAndOpacity(Color);
		return;
	}
	if (UBorder* const Br = Cast<UBorder>(SegmentWidget))
	{
		Br->SetBrushColor(Color);
		return;
	}
	if (UProgressBar* const Pb = Cast<UProgressBar>(SegmentWidget))
	{
		Pb->SetFillColorAndOpacity(Color);
		return;
	}
	if (UTextBlock* const Txt = Cast<UTextBlock>(SegmentWidget))
	{
		Txt->SetColorAndOpacity(Color);
		return;
	}
}

void USoundWaveformWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	RefreshSegmentCache();
	RefreshMeterVisual();
}

void USoundWaveformWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	RefreshSegmentCache();
	if (!IsValid(SoundMeterContainer))
	{
		UE_LOG(LogTemp, Warning, TEXT("SoundWaveformWidget: 未绑定名为 SoundMeterContainer 的面板，电平条不会显示。请在 Widget 蓝图中添加 HorizontalBox/VerticalBox 并命名为 SoundMeterContainer。"));
	}
	else if (CachedSegmentWidgets.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SoundWaveformWidget: SoundMeterContainer 下没有子控件，请在容器内添加与段数相等的 Image/Border 等作为每一格。"));
	}
	RefreshMeterVisual();
}

void USoundWaveformWidget::RefreshSegmentCache()
{
	CachedSegmentWidgets.Reset();
	if (!IsValid(SoundMeterContainer))
	{
		return;
	}

	CachedSegmentWidgets = SoundMeterContainer->GetAllChildren();
}

void USoundWaveformWidget::RefreshMeterVisual()
{
	const int32 Segments = CachedSegmentWidgets.Num();
	if (Segments <= 0)
	{
		return;
	}

	const float ScaledLevel = FMath::Max(0.0f, CurrentSoundLevel * Sensitivity);
	const float LitFloat = ScaledLevel * static_cast<float>(Segments);
	const int32 NumLit = (ScaledLevel >= 1.0f)
		? Segments
		: FMath::Clamp(FMath::FloorToInt(LitFloat), 0, Segments);

	for (int32 SegmentIdx = 0; SegmentIdx < Segments; ++SegmentIdx)
	{
		const bool bLit = SegmentIdx < NumLit;
		const int32 ChildIdx = bReverseSegmentChildMapping ? (Segments - 1 - SegmentIdx) : SegmentIdx;
		if (CachedSegmentWidgets.IsValidIndex(ChildIdx))
		{
			ApplySegmentStyle(CachedSegmentWidgets[ChildIdx], bLit, FilledSegmentColor, EmptySegmentColor);
		}
	}
}

void USoundWaveformWidget::UpdateSoundLevel(float InLevel)
{
	CurrentSoundLevel = FMath::Max(0.0f, InLevel);
	RefreshMeterVisual();
}

void USoundWaveformWidget::UpdateWaveform(const TArray<float>& WaveformData)
{
	if (WaveformData.Num() > 0)
	{
		UpdateSoundLevel(WaveformData.Last());
	}
	else
	{
		UpdateSoundLevel(0.0f);
	}
}

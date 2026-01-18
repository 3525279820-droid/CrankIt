// Fill out your copyright notice in the Description page of Project Settings.

#include "SoundWaveformWidget.h"
#include "Rendering/DrawElements.h"
#include "Styling/SlateBrush.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateNoResource.h"

void USoundWaveformWidget::UpdateWaveform(const TArray<float>& WaveformData)
{
	CurrentWaveformData = WaveformData;
	// 触发重绘
	if (IsValid(this))
	{
		Invalidate(EInvalidateWidget::Paint);
	}
}

int32 USoundWaveformWidget::NativePaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	// 调用父类绘制
	int32 MaxLayerId = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	// 绘制背景 - 使用颜色Brush
	FSlateColorBrush BackgroundBrush(BackgroundColor);
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		MaxLayerId++,
		AllottedGeometry.ToPaintGeometry(),
		&BackgroundBrush,
		ESlateDrawEffect::None,
		BackgroundColor
	);

	// 如果没有数据，直接返回
	if (CurrentWaveformData.Num() < 2)
	{
		return MaxLayerId;
	}

	// 获取绘制区域大小
	FVector2D WidgetSize = AllottedGeometry.GetLocalSize();
	float WidgetWidth = WidgetSize.X;
	float WidgetHeight = WidgetSize.Y;
	
	// 安全检查：如果Widget尺寸无效，直接返回
	if (WidgetWidth <= 0.0f || WidgetHeight <= 0.0f)
	{
		return MaxLayerId;
	}
	
	float CenterY = WidgetHeight * CenterLinePosition;

	// 计算每个数据点的X坐标间距
	float PointSpacing = WidgetWidth / (CurrentWaveformData.Num() - 1);

	// 绘制波形线
	TArray<FVector2D> Points;
	Points.Reserve(CurrentWaveformData.Num());

	for (int32 i = 0; i < CurrentWaveformData.Num(); i++)
	{
		float X = i * PointSpacing;
		// 将声音级别（0-1）映射到波形高度
		// 波形在中心线上下对称显示
		float Amplitude = CurrentWaveformData[i] * (WidgetHeight * 0.4f); // 使用40%的高度作为最大振幅
		float Y = CenterY - Amplitude; // 负值表示向上
		
		Points.Add(FVector2D(X, Y));
	}

	// 绘制波形线条
	if (Points.Num() >= 2)
	{
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			MaxLayerId++,
			AllottedGeometry.ToPaintGeometry(),
			Points,
			ESlateDrawEffect::None,
			WaveformColor,
			false, // 不闭合
			LineThickness
		);
	}

	// 绘制中心线（可选）
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		MaxLayerId++,
		AllottedGeometry.ToPaintGeometry(),
		TArray<FVector2D>{ FVector2D(0, CenterY), FVector2D(WidgetWidth, CenterY) },
		ESlateDrawEffect::None,
		FLinearColor(0.3f, 0.3f, 0.3f, 1.0f), // 灰色中心线
		false,
		1.0f
	);

	return MaxLayerId;
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoundWaveformWidget.generated.h"

/**
 * 声音波形显示Widget
 */
UCLASS()
class CRANKIT_API USoundWaveformWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 更新波形数据
	UFUNCTION(BlueprintCallable, Category = "Waveform")
	void UpdateWaveform(const TArray<float>& WaveformData);

protected:
	// 绘制波形
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, 
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, 
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	// 波形数据
	UPROPERTY(BlueprintReadOnly, Category = "Waveform")
	TArray<float> CurrentWaveformData;

	// 波形颜色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waveform")
	FLinearColor WaveformColor = FLinearColor::Green;

	// 背景颜色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waveform")
	FLinearColor BackgroundColor = FLinearColor::Black;

	// 波形线宽
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waveform")
	float LineThickness = 2.0f;

	// 波形中心线位置（0.0-1.0，0.5为居中）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waveform")
	float CenterLinePosition = 0.5f;
};

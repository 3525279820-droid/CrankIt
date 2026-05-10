// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/PanelWidget.h"
#include "SoundWaveformWidget.generated.h"

/**
 * 声音电平 Widget：在 UMG 设计器中用「容器 + 子控件」搭建每一段，本类只负责按电平更新各段样式。
 *
 * 用法：在 Widget 蓝图中放置与变量同名的面板（默认 HorizontalBox / VerticalBox 均可），
 * 例如命名为 SoundMeterContainer，其下每个**直接子控件**对应一格（支持 Image、Border、ProgressBar、TextBlock）。
 * 可在根上并列添加 Text、Image 等其它组件；电平条仅影响该容器内的子项。
 */
UCLASS()
class CRANKIT_API USoundWaveformWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Sound Meter")
	void UpdateSoundLevel(float InLevel);

	UFUNCTION(BlueprintCallable, Category = "Sound Meter")
	void UpdateWaveform(const TArray<float>& WaveformData);

	/** 重新收集 SoundMeterContainer 下的子控件（在运行时增删段后可在蓝图中调用）。 */
	UFUNCTION(BlueprintCallable, Category = "Sound Meter")
	void RefreshSegmentCache();

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;

	void RefreshMeterVisual();

	static void ApplySegmentStyle(UWidget* SegmentWidget, bool bLit, const FLinearColor& Filled, const FLinearColor& Empty);

	/** 与 UMG 中面板控件同名绑定；每个直接子控件为一格。 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> SoundMeterContainer;

	UPROPERTY(BlueprintReadOnly, Category = "Sound Meter")
	float CurrentSoundLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Meter", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float Sensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Meter")
	FLinearColor FilledSegmentColor = FLinearColor(0.2f, 0.85f, 0.25f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Meter")
	FLinearColor EmptySegmentColor = FLinearColor(0.12f, 0.12f, 0.12f, 1.0f);

	/**
	 * 为 true 时，逻辑上的第 0 段（最低电平）映射到容器的最后一个子控件。
	 * 用于 VerticalBox 自上而下建槽、但希望最下面一格最先被点亮等情况。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Meter")
	bool bReverseSegmentChildMapping = false;

private:
	TArray<UWidget*> CachedSegmentWidgets;
};

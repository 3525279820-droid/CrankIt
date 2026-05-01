#pragma once

/**
 * 薄 UI 层：只负责把子系统广播的 FText 显示到绑定控件上。
 * 字幕逻辑与时间轴全部在 USubtitleSubsystem，便于同一套数据驱动 HUD、过场 UI 等多处界面。
 */

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SubtitleWidget.generated.h"

class UTextBlock;

/** 构造时订阅、析构时取消订阅，避免 Subsystem 上挂着已销毁 Widget 的委托。 */
UCLASS()
class CRANKIT_API USubtitleWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleSubtitleLineChanged(FText InSubtitleLine);

	/** 蓝图里可放同名控件；纯 C++ CreateWidget 时由 NativeConstruct 自动创建。 */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* SubtitleText = nullptr;
};

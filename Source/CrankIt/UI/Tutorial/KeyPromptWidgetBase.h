// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KeyPromptWidgetBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKeyPromptDismissed);

UCLASS()
class CRANKIT_API UKeyPromptWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "KeyPrompt")
	FText PromptText;

	/** 提示显示时长（秒），可在 Widget 蓝图默认值或实例上调节。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeyPrompt", meta = (ClampMin = "0.1"))
	float DisplayDurationSeconds = 3.f;

	UPROPERTY(BlueprintAssignable, Category = "KeyPrompt")
	FOnKeyPromptDismissed OnDismissed;

	UFUNCTION(BlueprintCallable, Category = "KeyPrompt")
	virtual void SetupPrompt(FText InPrompt);

	UFUNCTION(BlueprintPure, Category = "KeyPrompt")
	bool IsPromptActive() const { return bIsPromptActive; }

	UFUNCTION(BlueprintCallable, Category = "KeyPrompt")
	void Dismiss();

	UFUNCTION(BlueprintImplementableEvent, Category = "KeyPrompt")
	void BP_OnPromptConfigured();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "KeyPrompt")
	void NotifyDismissed();

	void ApplyNonBlockingPresentation();
	void ClearAutoHideTimer();
	void ScheduleAutoHide();

	UFUNCTION()
	void OnAutoHideTimerFired();

	UPROPERTY(BlueprintReadOnly, Category = "KeyPrompt")
	bool bIsPromptActive = false;

	FTimerHandle AutoHideTimerHandle;
};

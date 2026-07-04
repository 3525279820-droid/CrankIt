// Fill out your copyright notice in the Description page of Project Settings.

#include "KeyPromptWidgetBase.h"

#include "TimerManager.h"

void UKeyPromptWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	ApplyNonBlockingPresentation();
	SetIsFocusable(false);
}

void UKeyPromptWidgetBase::NativeDestruct()
{
	ClearAutoHideTimer();
	Super::NativeDestruct();
}

void UKeyPromptWidgetBase::ApplyNonBlockingPresentation()
{
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (UWidget* const Root = GetRootWidget())
	{
		Root->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UKeyPromptWidgetBase::SetupPrompt(FText InPrompt)
{
	ClearAutoHideTimer();

	PromptText = InPrompt;
	bIsPromptActive = true;

	ApplyNonBlockingPresentation();
	BP_OnPromptConfigured();
	ScheduleAutoHide();
}

void UKeyPromptWidgetBase::ScheduleAutoHide()
{
	UWorld* const World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Duration = FMath::Max(0.1f, DisplayDurationSeconds);
	World->GetTimerManager().SetTimer(
		AutoHideTimerHandle,
		this,
		&UKeyPromptWidgetBase::OnAutoHideTimerFired,
		Duration,
		false);
}

void UKeyPromptWidgetBase::ClearAutoHideTimer()
{
	if (UWorld* const World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoHideTimerHandle);
	}
}

void UKeyPromptWidgetBase::OnAutoHideTimerFired()
{
	Dismiss();
}

void UKeyPromptWidgetBase::Dismiss()
{
	if (!bIsPromptActive)
	{
		return;
	}

	bIsPromptActive = false;
	ClearAutoHideTimer();
	SetVisibility(ESlateVisibility::Collapsed);

	OnDismissed.Broadcast();
}

void UKeyPromptWidgetBase::NotifyDismissed()
{
	Dismiss();
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SubtitleSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "SDTutorialWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTutorialDismissed);

/**
 * 
 */
UCLASS()
class CRANKIT_API USDTutorialWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void OnDismiss();

	UPROPERTY(BlueprintAssignable, Category = "Tutorial")
	FOnTutorialDismissed OnTutorialDismissed;

	virtual FReply NativeOnPreviewMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;
};

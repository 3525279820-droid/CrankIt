// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "SkipTutorialWidget.generated.h"

/**
 * 
 */
UCLASS()
class CRANKIT_API USkipTutorialWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	bool bSkipedTutorial = false;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Buttons")
	UButton* YesButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Buttons")
	UButton* NoButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Text")
	UTextBlock* SkipText = nullptr;

protected:
	UFUNCTION()
	void OnYesClicked();

	UFUNCTION()
	void OnNoClicked();

	UFUNCTION()
	void OnYesButtonHovered();

	UFUNCTION()
	void OnYesButtonUnHovered();

	
	void SetTextVisibility(UTextBlock* Text, bool bVisible);
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "CrankItAudioService.h"
#include "SkipTutorialWidget.generated.h"

class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnYesButtonClicked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNoButtonClicked);

/**
 * 
 */
UCLASS()
class CRANKIT_API USkipTutorialWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintAssignable, Category = "Tutorial")
	FOnYesButtonClicked YesButtonClicked;

	UPROPERTY(BlueprintAssignable, Category = "Tutorial")
	FOnNoButtonClicked NoButtonClicked;

	bool bSkipedTutorial = false;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Buttons")
	UButton* YesButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Buttons")
	UButton* NoButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Text")
	UTextBlock* SkipText = nullptr;

	UPROPERTY(EditAnywhere, Category = "SkipTutorial|Audio")
	TObjectPtr<USoundBase> BackgroundMusicSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "SkipTutorial|Audio")
	TObjectPtr<USoundBase> ButtonClickSound = nullptr;

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

	void PlayButtonClickSound();

	FCrankItSoundHandle BackgroundMusicHandle;
};

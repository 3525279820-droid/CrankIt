// Fill out your copyright notice in the Description page of Project Settings.

#include "SkipTutorialWidget.h"

#include "CrankItAudioService.h"

void USkipTutorialWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UCrankItAudioService* Audio = UCrankItAudioService::Get(this))
	{
		BackgroundMusicHandle = Audio->Play2DLoop(BackgroundMusicSound);
	}

	if (YesButton)
	{
		YesButton->OnClicked.AddDynamic(this, &USkipTutorialWidget::OnYesClicked);
		YesButton->OnHovered.AddDynamic(this, &USkipTutorialWidget::OnYesButtonHovered);
		YesButton->OnUnhovered.AddDynamic(this, &USkipTutorialWidget::OnYesButtonUnHovered);
	}
	if (NoButton)
	{
		NoButton->OnClicked.AddDynamic(this, &USkipTutorialWidget::OnNoClicked);
	}
}

void USkipTutorialWidget::NativeDestruct()
{
	if (UCrankItAudioService* Audio = UCrankItAudioService::Get(this))
	{
		Audio->Stop(BackgroundMusicHandle);
	}
	Super::NativeDestruct();
}

void USkipTutorialWidget::PlayButtonClickSound()
{
	if (UCrankItAudioService* Audio = UCrankItAudioService::Get(this))
	{
		Audio->Play2D(ButtonClickSound);
	}
}

void USkipTutorialWidget::OnYesClicked()
{
	PlayButtonClickSound();
	bSkipedTutorial = true;
	YesButtonClicked.Broadcast();
}

void USkipTutorialWidget::OnNoClicked()
{
	PlayButtonClickSound();
	bSkipedTutorial = false;
	NoButtonClicked.Broadcast();
}

void USkipTutorialWidget::OnYesButtonHovered()
{
	if(SkipText)
	{
		SetTextVisibility(SkipText, true);
	}
}

void USkipTutorialWidget::OnYesButtonUnHovered()
{
	if(SkipText)
	{
		SetTextVisibility(SkipText, false);
	}
}


void USkipTutorialWidget::SetTextVisibility(UTextBlock* Text, bool bVisible)
{
	if (!Text)
	{
		return;
	}
	Text->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

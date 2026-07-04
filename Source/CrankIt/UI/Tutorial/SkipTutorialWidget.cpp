// Fill out your copyright notice in the Description page of Project Settings.

#include "SkipTutorialWidget.h"

void USkipTutorialWidget::NativeConstruct()
{
	Super::NativeConstruct();

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

void USkipTutorialWidget::OnYesClicked()
{
	bSkipedTutorial = true;
	YesButtonClicked.Broadcast();
}

void USkipTutorialWidget::OnNoClicked()
{
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

// Fill out your copyright notice in the Description page of Project Settings.


#include "SDTutorialWidget.h"

#include "PlayerCamera.h"


void USDTutorialWidget::OnDismiss()
{
	OnTutorialDismissed.Broadcast();
	RemoveFromParent();
	
}

FReply USDTutorialWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if( InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		OnDismiss();
	}


	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

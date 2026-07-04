#include "MainMenuWidget.h"

#include "Components/Button.h"
#include "Components/VerticalBox.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartButton)
	{
		StartButton->OnClicked.RemoveDynamic(this, &UMainMenuWidget::OnStartButtonClicked);
		StartButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnStartButtonClicked);
	}
	if (OptionButton)
	{
		OptionButton->OnClicked.RemoveDynamic(this, &UMainMenuWidget::OnOptionButtonClicked);
		OptionButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnOptionButtonClicked);
	}
	if (CreditsButton)
	{
		CreditsButton->OnClicked.RemoveDynamic(this, &UMainMenuWidget::OnCreditsButtonClicked);
		CreditsButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnCreditsButtonClicked);
	}
	if (ExitButton)
	{
		ExitButton->OnClicked.RemoveDynamic(this, &UMainMenuWidget::OnExitButtonClicked);
		ExitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnExitButtonClicked);
	}

	// 初始默认收起子菜单，可在蓝图中按需调整。
	SetOptionSubMenuVisible(false);
	SetCreditsSubMenuVisible(false);
}

void UMainMenuWidget::SetOptionSubMenuVisible(bool bVisible)
{
	SetSubMenuVisibility(OptionSubMenu, bVisible);
}

void UMainMenuWidget::SetCreditsSubMenuVisible(bool bVisible)
{
	SetSubMenuVisibility(CreditsSubMenu, bVisible);
}

void UMainMenuWidget::ToggleOptionSubMenu()
{
	if (!OptionSubMenu)
	{
		return;
	}
	const bool bIsVisible = OptionSubMenu->GetVisibility() != ESlateVisibility::Collapsed;
	SetOptionSubMenuVisible(!bIsVisible);
}

void UMainMenuWidget::ToggleCreditsSubMenu()
{
	if (!CreditsSubMenu)
	{
		return;
	}
	const bool bIsVisible = CreditsSubMenu->GetVisibility() != ESlateVisibility::Collapsed;
	SetCreditsSubMenuVisible(!bIsVisible);
}

void UMainMenuWidget::OnStartButtonClicked()
{
	HandleStartClicked();
}

void UMainMenuWidget::OnOptionButtonClicked()
{
	ToggleOptionSubMenu();
	HandleOptionClicked();
}

void UMainMenuWidget::OnCreditsButtonClicked()
{
	ToggleCreditsSubMenu();
	HandleCreditsClicked();
}

void UMainMenuWidget::OnExitButtonClicked()
{
	HandleExitClicked();
}

void UMainMenuWidget::SetSubMenuVisibility(UWidget* MenuWidget, bool bVisible)
{
	if (!MenuWidget)
	{
		return;
	}
	MenuWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

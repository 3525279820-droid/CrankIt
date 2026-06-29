#include "TerminalMiniGameHost.h"

#include "TerminalWidget.h"
#include "TerminalDisplayController.h"
#include "ClassificationGameWidget.h"
#include "CalibrationWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "CrankItNarrativeIds.h"

void UTerminalMiniGameHost::Initialize(
	UTerminalWidget* InHost,
	UTerminalDisplayController* InDisplay,
	UClassificationGameWidget* InClassificationWidget,
	UCalibrationWidget* InCalibrationWidget)
{
	Host = InHost;
	Display = InDisplay;
	ClassificationGameWidget = InClassificationWidget;
	CalibrationWidget = InCalibrationWidget;
	CurrentInputMode = ETerminalInputMode::Terminal;
}

bool UTerminalMiniGameHost::RouteKey(const FKey& Key)
{
	if (CurrentInputMode == ETerminalInputMode::ClassificationGame)
	{
		if (Key == EKeys::Tab)
		{
			ExitClassificationGame();
			return true;
		}

		if (ClassificationGameWidget)
		{
			if (Key == EKeys::A || Key == EKeys::Left ||
				Key == EKeys::D || Key == EKeys::Right ||
				Key == EKeys::Enter)
			{
				ClassificationGameWidget->HandleKey(Key);
			}
		}
		return true;
	}

	if (CurrentInputMode == ETerminalInputMode::CalibrationGame)
	{
		if (Key == EKeys::Tab)
		{
			ExitCalibrationGame();
			return true;
		}

		if (CalibrationWidget && Key == EKeys::Enter)
		{
			CalibrationWidget->HandleKey(Key);
		}
		return true;
	}

	return false;
}

void UTerminalMiniGameHost::EnterClassificationGame()
{
	if (!Host || !Display)
	{
		return;
	}

	if (!ClassificationGameWidget)
	{
		Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::Classification_WidgetMissing);
		Display->StartDisplayingLines();
		CurrentInputMode = ETerminalInputMode::Terminal;
		return;
	}

	CurrentInputMode = ETerminalInputMode::ClassificationGame;
	Display->ClearInputLine();
	Display->UpdateDisplay();

	ClassificationGameWidget->SetVisibility(ESlateVisibility::Visible);

	TArray<FClassificationItem> DefaultItems;
	DefaultItems.Reserve(24);
	for (int32 i = 0; i < 24; ++i)
	{
		FClassificationItem Item;
		Item.Id = FName(*FString::Printf(TEXT("Item_%02d"), i + 1));
		if (i < 8)
		{
			Item.CorrectCategory = EClassificationCategory::Animal;
		}
		else if (i < 16)
		{
			Item.CorrectCategory = EClassificationCategory::Fruit;
		}
		else
		{
			Item.CorrectCategory = EClassificationCategory::Sport;
		}
		DefaultItems.Add(Item);
	}
	ClassificationGameWidget->StartGame(DefaultItems);

	ClassificationGameWidget->OnGameFinished.RemoveAll(this);
	ClassificationGameWidget->OnGameFinished.AddDynamic(this, &UTerminalMiniGameHost::HandleClassificationGameFinished);
}

void UTerminalMiniGameHost::HandleClassificationGameFinished(bool bAllCorrect)
{
	if (!Host)
	{
		return;
	}

	if (bAllCorrect)
	{
		Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::Classification_AllCorrect);
	}
	else
	{
		Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::Classification_Ended);
	}
	ExitClassificationGame();
}

void UTerminalMiniGameHost::ExitClassificationGame()
{
	if (!Host || !Display)
	{
		return;
	}

	CurrentInputMode = ETerminalInputMode::Terminal;
	Display->ClearInputLine();
	Display->UpdateDisplay();

	Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::Classification_BackToTerminal);
	Display->StartDisplayingLines();

	if (ClassificationGameWidget)
	{
		ClassificationGameWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UTerminalMiniGameHost::EnterCalibrationGame()
{
	if (!Host || !Display)
	{
		return;
	}

	if (!CalibrationWidget)
	{
		Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::Calibration_WidgetMissing);
		Display->StartDisplayingLines();
		CurrentInputMode = ETerminalInputMode::Terminal;
		return;
	}

	CurrentInputMode = ETerminalInputMode::CalibrationGame;
	Display->ClearInputLine();
	Display->UpdateDisplay();

	CalibrationWidget->SetVisibility(ESlateVisibility::Visible);

	if (UPanelSlot* PanelSlot = CalibrationWidget->Slot)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(PanelSlot))
		{
			CanvasSlot->SetZOrder(1000);
		}
	}

	CalibrationWidget->InvalidateLayoutAndVolatility();
	CalibrationWidget->OnGameFinished.RemoveAll(this);
	CalibrationWidget->OnGameFinished.AddDynamic(this, &UTerminalMiniGameHost::HandleCalibrationGameFinished);
	CalibrationWidget->StartGame();
}

void UTerminalMiniGameHost::HandleCalibrationGameFinished(bool bWon)
{
	if (!Host)
	{
		return;
	}

	if (bWon)
	{
		Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::Calibration_HumanVerified);
	}
	else
	{
		Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::Calibration_Ended);
	}
	ExitCalibrationGame();
}

void UTerminalMiniGameHost::ExitCalibrationGame()
{
	if (!Host || !Display)
	{
		return;
	}

	CurrentInputMode = ETerminalInputMode::Terminal;
	Display->ClearInputLine();
	Display->UpdateDisplay();

	Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::Calibration_BackToTerminal);
	Display->StartDisplayingLines();

	if (CalibrationWidget)
	{
		CalibrationWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

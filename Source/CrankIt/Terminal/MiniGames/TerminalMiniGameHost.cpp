#include "TerminalMiniGameHost.h"

#include "TerminalWidget.h"
#include "TerminalDisplayController.h"
#include "ClassificationGameWidget.h"
#include "CalibrationWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "CrankItNarrativeIds.h"

// 绑定 Host / Display 与子 Widget 引用
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

// 分类 / 校准模式下消费按键；Tab 退出当前小游戏
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

// 显示分类 Widget 并从 Content 目录随机抽图开局（见 ClassificationGameWidget::ImageContentPath）
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

	// StartGame 在无图时会立刻 Broadcast，须先绑定
	ClassificationGameWidget->OnGameFinished.RemoveAll(this);
	ClassificationGameWidget->OnGameFinished.AddDynamic(this, &UTerminalMiniGameHost::HandleClassificationGameFinished);
	ClassificationGameWidget->StartGame();
}

// 分类结束：写入 Narrative 输出块并退回终端模式
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

// 隐藏分类 Widget，恢复终端输入模式
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

// 显示校准 Widget 并 StartGame；抬高 ZOrder 以免被终端背景挡住
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

	// 避免终端 TextBlock 铺满 Canvas 时盖住校准子 Widget
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

// 校准结束：写入 Narrative 输出块并退回终端模式
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

// 隐藏校准 Widget，恢复终端输入模式
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

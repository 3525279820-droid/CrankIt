#pragma once

// 叙事 / 终端输出在 Primary Data Asset 中使用的 TrackId、BlockId（编辑器填表时须完全一致）

#include "CoreMinimal.h"

namespace CrankItNarrative
{
	// 字幕轨 ID（UCrankItNarrativeData::SubtitleTracks）
	namespace Subtitle
	{
		inline const FName Intro_SkipTutorialPrompt(TEXT("Intro_SkipTutorialPrompt"));
		inline const FName TutorialSkipped(TEXT("TutorialSkipped"));
		inline const FName TutorialNotSkipped_GordonIntro(TEXT("TutorialNotSkipped_GordonIntro"));
		inline const FName PostTutorial(TEXT("PostTutorial"));
		inline const FName ChargeTutorial_Level1(TEXT("ChargeTutorial_Level1"));
		inline const FName ChargeTutorial_Level2(TEXT("ChargeTutorial_Level2"));
		inline const FName ChargeTutorial_Level3(TEXT("ChargeTutorial_Level3"));
		inline const FName EMP_Tutorial(TEXT("EMP_Tutorial"));
	}

	// 终端输出块 ID（UCrankItTerminalCommandData::OutputBlocks）
	namespace Terminal
	{
		inline const FName ComputerScreen_Startup(TEXT("ComputerScreen_Startup"));
		inline const FName ComputerScreen_Destination(TEXT("ComputerScreen_Destination"));
		inline const FName ComputerScreen_FirstPrompt(TEXT("ComputerScreen_FirstPrompt"));

		inline const FName Boordle_GuessCorrectFollowup(TEXT("Boordle_GuessCorrectFollowup"));
		inline const FName Boordle_OutOfGuessesPrefix(TEXT("Boordle_OutOfGuessesPrefix"));
		inline const FName ScanRepair_Finished(TEXT("ScanRepair_Finished"));
		inline const FName Boordle_Started(TEXT("Boordle_Started"));
		inline const FName Calibrate_Started(TEXT("Calibrate_Started"));
		inline const FName UpdateSystem_Warning(TEXT("UpdateSystem_Warning"));
		inline const FName LiftQuarantine_Warning(TEXT("LiftQuarantine_Warning"));
		inline const FName CalibrateNorthEntry_Unlocking(TEXT("CalibrateNorthEntry_Unlocking"));
		inline const FName CalibrateNorthEntry_Unlocked(TEXT("CalibrateNorthEntry_Unlocked"));
		inline const FName CalibrateNorthEntry_LowAux(TEXT("CalibrateNorthEntry_LowAux"));
		inline const FName CalibrateNorthEntry_Restricted(TEXT("CalibrateNorthEntry_Restricted"));
		inline const FName CalibrateNorthEntry_Countdown30(TEXT("CalibrateNorthEntry_Countdown30"));
		inline const FName CalibrateNorthEntry_UnlockedReady(TEXT("CalibrateNorthEntry_UnlockedReady"));
		inline const FName GodIsDead(TEXT("GodIsDead"));
		inline const FName LiftOperational(TEXT("LiftOperational"));
		inline const FName Ascend_GameOver(TEXT("Ascend_GameOver"));

		inline const FName Classification_AllCorrect(TEXT("Classification_AllCorrect"));
		inline const FName Classification_Ended(TEXT("Classification_Ended"));
		inline const FName Classification_BackToTerminal(TEXT("Classification_BackToTerminal"));
		inline const FName Classification_WidgetMissing(TEXT("Classification_WidgetMissing"));

		inline const FName Calibration_HumanVerified(TEXT("Calibration_HumanVerified"));
		inline const FName Calibration_Ended(TEXT("Calibration_Ended"));
		inline const FName Calibration_BackToTerminal(TEXT("Calibration_BackToTerminal"));
		inline const FName Calibration_WidgetMissing(TEXT("Calibration_WidgetMissing"));

		inline const FName Battery_TransferInProgress(TEXT("Battery_TransferInProgress"));
		inline const FName Battery_TransferAlreadyActive(TEXT("Battery_TransferAlreadyActive"));
		inline const FName Battery_LowPowerError(TEXT("Battery_LowPowerError"));
		inline const FName Battery_TransferStarted(TEXT("Battery_TransferStarted"));
		inline const FName Battery_TransferComplete(TEXT("Battery_TransferComplete"));
		inline const FName Battery_TransferInterrupted(TEXT("Battery_TransferInterrupted"));
	}
}

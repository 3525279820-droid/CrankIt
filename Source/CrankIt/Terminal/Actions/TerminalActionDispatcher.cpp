#include "TerminalActionDispatcher.h"

#include "TerminalWidget.h"
#include "TerminalDisplayController.h"
#include "CrankItNarrativeSubsystem.h"
#include "CrankItNarrativeIds.h"
#include "CrankItTerminalActionIds.h"
#include "DoubleAutoDoor.h"
#include "FlashTopLight.h"
#include "Monster.h"
#include "CrankItActorRegistry.h"

// 绑定 Host / Display 并注册全部 ActionId 处理函数
void UTerminalActionDispatcher::Initialize(UTerminalWidget* InHost, UTerminalDisplayController* InDisplay)
{
	Host = InHost;
	Display = InDisplay;
	RegisterActions();
}

// 注册各 ActionId 对应的副作用
void UTerminalActionDispatcher::RegisterActions()
{
	ActionHandlers.Empty();

	ActionHandlers.Add(CrankItTerminalAction::OpenDoor, [this](const FString&, const TMap<FString, TArray<FString>>*)
	{
		// 经 ActorRegistry 触发双开门
		if (UCrankItActorRegistry* Reg = GetWorld()->GetSubsystem<UCrankItActorRegistry>())
		{
			if (ADoubleAutoDoor* Door = Reg->GetDoubleAutoDoor())
			{
				Door->DoorOpened();
			}
		}
	});

	ActionHandlers.Add(CrankItTerminalAction::ClearTerminal, [this](const FString&, const TMap<FString, TArray<FString>>*)
	{
		if (Display)
		{
			Display->ClearTerminal();
		}
	});

	ActionHandlers.Add(CrankItTerminalAction::Reboot, [this](const FString& CommandKey, const TMap<FString, TArray<FString>>* CommandTextMap)
	{
		if (!Display || !Host)
		{
			return;
		}
		AppendCommandTextFromMap(CommandKey, CommandTextMap);

		Display->StartDisplayingLinesProcedure(3.f, [this]()
		{
			// REBOOT 动画结束后经 ActorRegistry 调用 EnableSpawning 与顶灯闪烁
			if (UCrankItActorRegistry* Reg = GetWorld()->GetSubsystem<UCrankItActorRegistry>())
			{
				if (AMonster* Monster = Reg->GetMonster())
				{
					Monster->EnableSpawning();
				}
				if (AFlashTopLight* FlashLight = Reg->GetFlashTopLight())
				{
					FlashLight->SetIntensity(1000.f);
					FlashLight->LightStartFlash(.05f, 5);
				}
			}
		});
	});

	ActionHandlers.Add(CrankItTerminalAction::ScanAndRepair, [this](const FString& CommandKey, const TMap<FString, TArray<FString>>* CommandTextMap)
	{
		if (!Display || !Host)
		{
			return;
		}
		AppendCommandTextFromMap(CommandKey, CommandTextMap);
		Display->StartDisplayingLines();
		Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::ScanRepair_Finished);
		Display->StartDisplayingLinesProcedure(1.f);
	});

	ActionHandlers.Add(CrankItTerminalAction::Boordle, [this](const FString&, const TMap<FString, TArray<FString>>*)
	{
		if (!Display || !Host)
		{
			return;
		}
		Host->StartBoardleGame();
		Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::Boordle_Started);
		Display->StartDisplayingLines();
	});

	ActionHandlers.Add(CrankItTerminalAction::EnterClassificationGame, [this](const FString&, const TMap<FString, TArray<FString>>*)
	{
		if (Host)
		{
			Host->EnterClassificationGame();
		}
	});

	ActionHandlers.Add(CrankItTerminalAction::Calibrate, [this](const FString&, const TMap<FString, TArray<FString>>*)
	{
		if (!Display || !Host)
		{
			return;
		}
		Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::Calibrate_Started);
		Display->StartDisplayingLines();
		Host->EnterCalibrationGame();
	});

	ActionHandlers.Add(CrankItTerminalAction::UpdateSystem, [this](const FString&, const TMap<FString, TArray<FString>>*)
	{
		if (!Display || !Host)
		{
			return;
		}
		Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::UpdateSystem_Warning);
		Display->StartDisplayingLinesProcedure(5.f);
	});

	ActionHandlers.Add(CrankItTerminalAction::LiftQuarantine, [this](const FString&, const TMap<FString, TArray<FString>>*)
	{
		if (!Display || !Host)
		{
			return;
		}
		Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::LiftQuarantine_Warning);
		Display->StartDisplayingLinesProcedure(5.f);
	});

	ActionHandlers.Add(CrankItTerminalAction::CalibrateNorthEntryDoor, [this](const FString&, const TMap<FString, TArray<FString>>*)
	{
		if (!Display || !Host)
		{
			return;
		}

		Host->EnterCalibrationGame([this](bool bWon)
		{
			if (!Display || !Host)
			{
				return;
			}
			if (!bWon)
			{
				Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::Calibration_Ended);
				Display->StartDisplayingLines();
				Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::Calibration_BackToTerminal);
				Display->StartDisplayingLines();
				return;
			}

			// 与 OpenDoor Action 相同：经 ActorRegistry 开门（音效/北向解锁在门内完成）
			if (UWorld* World = GetWorld())
			{
				if (UCrankItActorRegistry* Reg = World->GetSubsystem<UCrankItActorRegistry>())
				{
					if (ADoubleAutoDoor* Door = Reg->GetDoubleAutoDoor())
					{
						Door->DoorOpened();
					}
				}
			}

			PlayCalibrateNorthEntryDoorSequence();
		});
	});

	ActionHandlers.Add(CrankItTerminalAction::GodIsDead, [this](const FString&, const TMap<FString, TArray<FString>>*)
	{
		if (!Display || !Host)
		{
			return;
		}
		TArray<FString> GodLines;
		if (UWorld* World = GetWorld())
		{
			if (UCrankItNarrativeSubsystem* Narrative = World->GetSubsystem<UCrankItNarrativeSubsystem>())
			{
				Narrative->GetTerminalOutputLines(CrankItNarrative::Terminal::GodIsDead, GodLines);
			}
		}
		const int32 LineCount = GodLines.Num() > 0 ? GodLines.Num() : 14;
		for (int32 i = 0; i < LineCount; ++i)
		{
			if (GodLines.IsValidIndex(i))
			{
				Display->AddPendingLine(GodLines[i]);
			}
			else
			{
				// Data Asset 未配置时 fallback
				Display->AddPendingLine(TEXT("GOD IS DEAD"));
			}
			Display->StartDisplayingLinesProcedure(1.f);
		}
	});

	ActionHandlers.Add(CrankItTerminalAction::LiftOperational, [this](const FString&, const TMap<FString, TArray<FString>>*)
	{
		if (!Display || !Host)
		{
			return;
		}
		Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::LiftOperational);
		Display->StartDisplayingLines();
	});

	ActionHandlers.Add(CrankItTerminalAction::Ascend, [this](const FString&, const TMap<FString, TArray<FString>>*)
	{
		if (!Display || !Host)
		{
			return;
		}
		Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::Ascend_GameOver);
		Display->StartDisplayingLines();
	});
}

void UTerminalActionDispatcher::PlayCalibrateNorthEntryDoorSequence() const
{
	if (!Display || !Host)
	{
		return;
	}
	
	// 测试代码，没有真正的等待效果

	Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::CalibrateNorthEntry_Unlocking);
	Display->StartDisplayingLinesProcedure(15.f);
	Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::CalibrateNorthEntry_Unlocked);
	Display->StartDisplayingLines();
	Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::CalibrateNorthEntry_LowAux);
	Display->StartDisplayingLinesProcedure(5.f);
	Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::CalibrateNorthEntry_Restricted);
	Display->StartDisplayingLines();
	Display->ClearTerminal();
	Display->ClearPendingLines();
	Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::CalibrateNorthEntry_Countdown30);
	Display->StartDisplayingLinesProcedure(30.f);
	Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::CalibrateNorthEntry_UnlockedReady);
	Display->StartDisplayingLinesProcedure(30.f);
	Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::LiftOperational);
	Display->StartDisplayingLines();
}

// 从 Router 持有的 CommandTextMap 追加同步文案（REBOOT / SCAN AND REPAIR 等）
void UTerminalActionDispatcher::AppendCommandTextFromMap(
	const FString& CommandKey,
	const TMap<FString, TArray<FString>>* CommandTextMap) const
{
	if (!Display || !CommandTextMap)
	{
		return;
	}
	if (const TArray<FString>* Lines = CommandTextMap->Find(CommandKey))
	{
		Display->AppendPendingLines(*Lines);
	}
}

// 按 ActionId 查找并执行已注册的副作用处理函数
void UTerminalActionDispatcher::Execute(
	FName ActionId,
	const FString& CommandKey,
	const TMap<FString, TArray<FString>>* CommandTextMap)
{
	if (ActionId.IsNone())
	{
		return;
	}

	if (const TFunction<void(const FString&, const TMap<FString, TArray<FString>>*)>* Handler = ActionHandlers.Find(ActionId))
	{
		(*Handler)(CommandKey, CommandTextMap);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TerminalActionDispatcher: 未注册的 ActionId '%s'"), *ActionId.ToString());
	}
}

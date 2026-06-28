#include "TerminalWidget.h"

#include "Battery.h"
#include "BatteryPowerChecker.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerCamera.h"
#include "CalibrationWidget.h"
#include "ClassificationGameWidget.h"
#include "DoubleAutoDoor.h"
#include "CrankItNarrativeSubsystem.h"
#include "CrankItNarrativeIds.h"

void UTerminalWidget::GenerateTarget()
{
	NewTarget.Empty();
	for (int32 i = 0; i < BinaryLength; ++i)
	{
		const bool b = FMath::RandBool();
		NewTarget += b ? TEXT("1") : TEXT("0");
	}
}

void UTerminalWidget::ResetRound()
{
	GuessHistory.Empty();
	CurrentGuessCount = 0;
	GenerateTarget();
}

// 从 Data Asset 读取终端输出块并追加到显示队列
void UTerminalWidget::AppendTerminalOutputBlock(FName BlockId)
{
	if (!Display)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UCrankItNarrativeSubsystem* Narrative = World->GetSubsystem<UCrankItNarrativeSubsystem>())
		{
			TArray<FString> Lines;
			// 从 Data Asset 按 BlockId 取文案，压入显示队列
			if (Narrative->GetTerminalOutputLines(BlockId, Lines))
			{
				Display->AppendPendingLines(Lines);
			}
		}
	}
}

// 从 TerminalCommandData 加载命令顺序与 CommandTextMap
void UTerminalWidget::LoadTerminalCommandDataFromAsset()
{
	CommandSequence.Empty();
	CommandTextMap.Empty();
	NextCommandIndex = 0;

	if (UWorld* World = GetWorld())
	{
		if (UCrankItNarrativeSubsystem* Narrative = World->GetSubsystem<UCrankItNarrativeSubsystem>())
		{
			if (Narrative->ApplyTerminalCommandData(CommandSequence, CommandTextMap))
			{
				return;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("TerminalWidget: 未加载 TerminalCommandData，请在 GameMode 上指定 DA_CrankItTerminalCommands。"));
}

// 注册各命令的行为逻辑（副作用）；文案由 Data Asset / AppendTerminalOutputBlock 提供
void UTerminalWidget::SetupCommandActions()
{
	CommandActionMap.Empty();

	CommandActionMap.Add(TEXT("O"), [this]()
	{
		if (ADoubleAutoDoor* Door = Cast<ADoubleAutoDoor>(UGameplayStatics::GetActorOfClass(GetWorld(), ADoubleAutoDoor::StaticClass())))
		{
			Door->DoorOpened();
		}
	});
	CommandActionMap.Add(TEXT("CLEAR"), [this]()
	{
		if (Display)
		{
			Display->ClearTerminal();
		}
	});

	CommandActionMap.Add(TEXT("REBOOT"), [this]()
	{
		if (!Display)
		{
			return;
		}
		if (const TArray<FString>* Lines = CommandTextMap.Find(PendingCommandKey))
		{
			Display->AppendPendingLines(*Lines);
		}

		Display->StartDisplayingLinesProcedure(3.f, [this]()
		{
			// REBOOT 动画结束后：启用怪物生成与顶灯闪烁
			if (AMonster* Monster = Cast<AMonster>(UGameplayStatics::GetActorOfClass(GetWorld(), AMonster::StaticClass())))
			{
				Monster->bSpawnable = true;
			}
			if (AFlashTopLight* FlashLight = Cast<AFlashTopLight>(UGameplayStatics::GetActorOfClass(GetWorld(), AFlashTopLight::StaticClass())))
			{
				FlashLight->SetIntensity(1000.f);
				FlashLight->LightStartFlash(.05f, 5);
			}
		});
	});

	CommandActionMap.Add(TEXT("SCAN AND REPAIR"), [this]()
	{
		if (!Display)
		{
			return;
		}
		if (const TArray<FString>* Lines = CommandTextMap.Find(PendingCommandKey))
		{
			Display->AppendPendingLines(*Lines);
		}
		Display->StartDisplayingLines();
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::ScanRepair_Finished);
		Display->StartDisplayingLinesProcedure(1.f);
	});

	CommandActionMap.Add(TEXT("BOORDLE"), [this]()
	{
		if (!Display)
		{
			return;
		}
		BinaryGuessActivated = true;
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::Boordle_Started);
		Display->StartDisplayingLines();
	});

	CommandActionMap.Add(TEXT(""), [this]()
	{
		EnterClassificationGame();
	});

	CommandActionMap.Add(TEXT("CALIBRATE"), [this]()
	{
		if (!Display)
		{
			return;
		}
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::Calibrate_Started);
		Display->StartDisplayingLines();
		EnterCalibrationGame();
	});

	CommandActionMap.Add(TEXT("UPDATE SYSTEM"), [this]()
	{
		if (!Display)
		{
			return;
		}
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::UpdateSystem_Warning);
		Display->StartDisplayingLinesProcedure(5.f);
	});

	CommandActionMap.Add(TEXT("LIFT QUARANTINE"), [this]()
	{
		if (!Display)
		{
			return;
		}
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::LiftQuarantine_Warning);
		Display->StartDisplayingLinesProcedure(5.f);
	});

	CommandActionMap.Add(TEXT("CALIBRATE NORTH ENTRY DOOR"), [this]()
	{
		if (!Display)
		{
			return;
		}
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::CalibrateNorthEntry_Unlocking);
		Display->StartDisplayingLinesProcedure(15.f);
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::CalibrateNorthEntry_Unlocked);
		Display->StartDisplayingLines();
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::CalibrateNorthEntry_LowAux);
		Display->StartDisplayingLinesProcedure(5.f);
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::CalibrateNorthEntry_Restricted);
		Display->StartDisplayingLines();
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::CalibrateNorthEntry_Countdown30);
		Display->StartDisplayingLinesProcedure(30.f);
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::CalibrateNorthEntry_UnlockedReady);
		Display->StartDisplayingLinesProcedure(30.f);
	});

	CommandActionMap.Add(TEXT(""), [this]()
	{
		if (!Display)
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

	CommandActionMap.Add(TEXT(""), [this]()
	{
		if (!Display)
		{
			return;
		}
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::LiftOperational);
		Display->StartDisplayingLines();
	});

	CommandActionMap.Add(TEXT("ASCEND"), [this]()
	{
		if (!Display)
		{
			return;
		}
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::Ascend_GameOver);
		Display->StartDisplayingLines();
	});
}

void UTerminalWidget::AddToGuessHistory(const FString& Guess)
{
	GuessHistory.Add(Guess);
	CurrentGuessCount++;

	int32 MatchCount = 0;
	for (int32 i = 0; i < BinaryLength; ++i)
	{
		if (Guess[i] == NewTarget[i])
		{
			MatchCount++;
		}
	}

	if (!Display)
	{
		return;
	}

	Display->AddPendingLine(FString::Printf(TEXT("Guess %d: %s  (matching bits: %d)"), CurrentGuessCount, *Guess, MatchCount));
	if (MatchCount == BinaryLength)
	{
		BinaryGuessActivated = false;
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::Boordle_GuessCorrectFollowup);
		Display->StartDisplayingLines();
	}
	if (CurrentGuessCount >= MaxGuesses)
	{
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::Boordle_OutOfGuessesPrefix);
		// 目标串为运行时生成，保留在代码中
		Display->AddPendingLine(FString::Printf(TEXT("Target decimal was: %s"), *NewTarget));
		Display->AddPendingLine(TEXT("Starting new round..."));
		Display->StartDisplayingLines();
		Display->SetOneShotTimer(2.5f, [this]()
		{
			ResetRound();
		});
	}
	Display->StartDisplayingLines();
}

void UTerminalWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 显示层与 Widget 解耦，由 Display 负责逐行输出
	Display = NewObject<UTerminalDisplayController>(this);
	Display->Initialize(this, TerminalText);
	Display->Interval = Interval;
	Display->ClearInputLine();

	SetIsFocusable(true);

	BinaryLength = 6;
	MaxGuesses = 7;
	ResetRound();

	// 命令表来自 Data Asset；行为表在此注册
	LoadTerminalCommandDataFromAsset();
	SetupCommandActions();

	/* LEGACY — CommandSequence / CommandTextMap / CommandActionMap 内联文案（已迁至 UCrankItTerminalCommandData）
	CommandSequence = {
		TEXT("REBOOT"),
		TEXT("SCAN AND REPAIR"),
		TEXT("BOORDLE"),
		TEXT("I AM A HUMAN BEING"),
		TEXT("LIFT QUARANTINE"),
		TEXT("CALIBRATE NORTH ENTRY DOOR"),
		TEXT("ASCEND"),
	};
	NextCommandIndex = 0;

	CommandTextMap.Add(TEXT("JUMP"), { TEXT("Hello world!"), TEXT("compile..."), TEXT("Done!") });
	CommandTextMap.Add(TEXT("HELLO"), { TEXT("Hi!"), TEXT("Welcome.") });
	CommandTextMap.Add(TEXT("REBOOT"), {
		TEXT("cOS [Version 12.0.19248.417] "),
		TEXT("(c) 1998 JTEC corporation. All rights reserved."),
		TEXT(""),
		TEXT("safe mode is active "),
		TEXT("An unexpected crash has occurred! "),
		TEXT(" to fix potential faults type: "),
		TEXT("SCAN AND REPAIR")
	});
	CommandTextMap.Add(TEXT("SCAN AND REPAIR"), {
		TEXT(""),
		TEXT("Scanning Files "),
		TEXT(" "),
		TEXT("Checking BIOS ...OK"),
		TEXT("Checking OS ...OK"),
		TEXT("Checking Data ...OK"),
	});
	CommandTextMap.Add(TEXT("I AM A HUMAN BEING"), {
		TEXT(""),
		TEXT("insufficient further validation required."),
		TEXT("press any key to start additional verification."),
	});
	CommandTextMap.Add(TEXT("LIFT QUARANTINE"),
	{
		TEXT(""),
		TEXT("ERROR!"),
		TEXT("NORTH ENTRY door is not calibrated correctly"),
		TEXT(""),
		TEXT("to calibrate door type"),
		TEXT("\"CALIBRATE NORTH ENTRY DOOR\"")
	});
	// … CommandActionMap 内 AppendPendingLines 文案见 CrankItNarrativeIds.h → Terminal::* BlockId
	*/
}

void UTerminalWidget::StartDisplayingLinesProcedure(float Delay, TFunction<void()> OnProcedureComplete)
{
	if (Display)
	{
		Display->StartDisplayingLinesProcedure(Delay, MoveTemp(OnProcedureComplete));
	}
}

void UTerminalWidget::NativeDestruct()
{
	if (Display)
	{
		Display->ClearDisplayTimer();
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BatteryHoldTimerHandle);
	}
	bBatteryHoldActive = false;
	Super::NativeDestruct();
}

void UTerminalWidget::AddNewLine(const FString& Line)
{
	if (Display)
	{
		Display->AddNewLine(Line);
	}
}

void UTerminalWidget::AddNewLines(const TArray<FString>& Lines)
{
	if (Display)
	{
		Display->AddNewLines(Lines);
	}
}

FReply UTerminalWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (bBatteryHoldActive)
	{
		if (Key == EKeys::Tab)
		{
			if (UWorld* World = GetWorld())
			{
				if (APlayerController* PC = World->GetFirstPlayerController())
				{
					if (APawn* Pawn = PC->GetPawn())
					{
						if (APlayerCamera* PlayerCamera = Cast<APlayerCamera>(Pawn))
						{
							PlayerCamera->ExitScreenInput(FInputActionValue());
						}
					}
				}
			}
		}
		return FReply::Handled();
	}

	if (CurrentInputMode == ETerminalInputMode::ClassificationGame)
	{
		if (Key == EKeys::Tab)
		{
			ExitClassificationGame();
			return FReply::Handled();
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
		return FReply::Handled();
	}

	if (CurrentInputMode == ETerminalInputMode::CalibrationGame)
	{
		if (Key == EKeys::Tab)
		{
			ExitCalibrationGame();
			return FReply::Handled();
		}

		if (CalibrationWidget && Key == EKeys::Enter)
		{
			CalibrationWidget->HandleKey(Key);
		}
		return FReply::Handled();
	}

	if (!Display)
	{
		return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
	}

	if (Key == EKeys::Enter)
	{
		CommitInput();
		return FReply::Handled();
	}
	if (Key == EKeys::BackSpace)
	{
		Display->RemoveLastInputChar();
		Display->UpdateDisplay();
		return FReply::Handled();
	}
	if (Key == EKeys::Tab)
	{
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				if (APawn* Pawn = PC->GetPawn())
				{
					if (APlayerCamera* PlayerCamera = Cast<APlayerCamera>(Pawn))
					{
						PlayerCamera->ExitScreenInput(FInputActionValue());
					}
				}
			}
		}
		return FReply::Handled();
	}

	const uint32 CharCode = InKeyEvent.GetCharacter();
	if (CharCode != 0)
	{
		const TCHAR Char = static_cast<TCHAR>(CharCode);
		if (BinaryGuessActivated)
		{
			if ((Char == TEXT('0') || Char == TEXT('1')) && Display->GetCurrentInputLine().Len() < BinaryLength)
			{
				Display->AppendCharToInputLine(Char);
				CommitGuessNum();
				Display->UpdateDisplay();
			}
		}
		else
		{
			Display->AppendCharToInputLine(Char);
			Display->UpdateDisplay();
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UTerminalWidget::CommitGuessNum()
{
	if (!Display || Display->GetCurrentInputLine().Len() != BinaryLength)
	{
		return;
	}

	const FString Guess = Display->GetCurrentInputLine();
	Display->AppendToCurrentText(Guess + TEXT("\n"));
	AddToGuessHistory(Guess);
	Display->ClearInputLine();
	Display->UpdateDisplay();
}

void UTerminalWidget::ClearPreviousWrongAttemptFromScreen()
{
	if (!Display)
	{
		return;
	}

	Display->ClearDisplayTimer();
	Display->RemovePendingLinesIf([](const FString& Line)
	{
		return Line.StartsWith(TEXT("Unknown command:"));
	});

	if (!CurrentStageErrorText.IsEmpty() && Display->CurrentTextEndsWith(CurrentStageErrorText))
	{
		Display->RemoveCurrentTextSuffix(CurrentStageErrorText);
	}
	CurrentStageErrorText.Empty();

	if (!CurrentStageWrongInputText.IsEmpty() && Display->CurrentTextEndsWith(CurrentStageWrongInputText))
	{
		Display->RemoveCurrentTextSuffix(CurrentStageWrongInputText);
	}
	CurrentStageWrongInputText.Empty();
}

void UTerminalWidget::ShowStageCommandError(const FString& SubmittedInput)
{
	if (!Display)
	{
		return;
	}

	const FString ErrorLine = FString::Printf(TEXT("Unknown command: %s"), *SubmittedInput);
	CurrentStageErrorText = ErrorLine + TEXT("\n");
	Display->AddPendingLine(ErrorLine);
	bErrorShownForCurrentCommand = true;
	Display->StartDisplayingLines();
}

void UTerminalWidget::CommitInput()
{
	if (!Display)
	{
		return;
	}

	const FString SubmittedInput = Display->GetCurrentInputLine();
	Display->ClearInputLine();

	if (bBatteryHoldActive)
	{
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::Battery_TransferInProgress);
		/* LEGACY: TEXT("Auxiliary power transfer in progress. Please wait...") */
		Display->StartDisplayingLines();
		Display->UpdateDisplay();
		return;
	}

	auto HandleWrongCommand = [this, &SubmittedInput]()
	{
		if (bErrorShownForCurrentCommand || !CurrentStageWrongInputText.IsEmpty())
		{
			ClearPreviousWrongAttemptFromScreen();
		}
		CurrentStageWrongInputText = SubmittedInput + TEXT("\n");
		Display->AppendToCurrentText(CurrentStageWrongInputText);
		ShowStageCommandError(SubmittedInput);
	};

	if (NextCommandIndex >= CommandSequence.Num())
	{
		HandleWrongCommand();
		Display->UpdateDisplay();
		return;
	}

	const FString& Expected = CommandSequence[NextCommandIndex];
	UE_LOG(LogTemp, Display, TEXT("now index: %d"), NextCommandIndex);

	if (SubmittedInput.Equals(Expected))
	{
		Display->ClearDisplayTimer();
		Display->ClearPendingLines();
		Display->ClearTerminal();
		bErrorShownForCurrentCommand = false;
		CurrentStageErrorText.Empty();
		CurrentStageWrongInputText.Empty();

		if (CommandActionMap.Contains(SubmittedInput))
		{
			PendingCommandKey = SubmittedInput;
			CommandActionMap[SubmittedInput]();
			PendingCommandKey.Empty();
			NextCommandIndex++;
		}
		else if (CommandTextMap.Contains(SubmittedInput))
		{
			Display->AppendPendingLines(CommandTextMap[SubmittedInput]);
			Display->StartDisplayingLines();
			NextCommandIndex++;
		}
		else
		{
			Display->AddPendingLine(FString::Printf(TEXT("Unknown command: %s"), *SubmittedInput));
			Display->StartDisplayingLines();
		}
	}
	else
	{
		HandleWrongCommand();
	}

	Display->UpdateDisplay();
}

void UTerminalWidget::EnterClassificationGame()
{
	if (!Display)
	{
		return;
	}

	if (!ClassificationGameWidget)
	{
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::Classification_WidgetMissing);
		/* LEGACY: ClassificationGameWidget is not bound... */
		Display->StartDisplayingLines();
		bInClassificationGame = false;
		CurrentInputMode = ETerminalInputMode::Terminal;
		return;
	}

	bInClassificationGame = true;
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
	ClassificationGameWidget->OnGameFinished.AddDynamic(this, &UTerminalWidget::HandleClassificationGameFinished);
}

void UTerminalWidget::HandleClassificationGameFinished(bool bAllCorrect)
{
	if (!Display)
	{
		return;
	}

	if (bAllCorrect)
	{
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::Classification_AllCorrect);
		/* LEGACY: TEXT("All classifications correct.") */
	}
	else
	{
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::Classification_Ended);
		/* LEGACY: TEXT("Classification game ended.") */
	}
	ExitClassificationGame();
}

void UTerminalWidget::ExitClassificationGame()
{
	if (!Display)
	{
		return;
	}

	bInClassificationGame = false;
	CurrentInputMode = ETerminalInputMode::Terminal;
	Display->ClearInputLine();
	Display->UpdateDisplay();

	AppendTerminalOutputBlock(CrankItNarrative::Terminal::Classification_BackToTerminal);
	/* LEGACY: TEXT("Classification game finished. Back to terminal.") */
	Display->StartDisplayingLines();

	if (ClassificationGameWidget)
	{
		ClassificationGameWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UTerminalWidget::EnterCalibrationGame()
{
	if (!Display)
	{
		return;
	}

	if (!CalibrationWidget)
	{
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::Calibration_WidgetMissing);
		/* LEGACY: CalibrationWidget is not bound... */
		Display->StartDisplayingLines();
		CurrentInputMode = ETerminalInputMode::Terminal;
		return;
	}

	bInClassificationGame = false;
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
	CalibrationWidget->OnGameFinished.AddDynamic(this, &UTerminalWidget::HandleCalibrationGameFinished);
	CalibrationWidget->StartGame();
}

void UTerminalWidget::HandleCalibrationGameFinished(bool bWon)
{
	if (!Display)
	{
		return;
	}

	if (bWon)
	{
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::Calibration_HumanVerified);
		/* LEGACY: HUMAN BEING verified / UPDATE SYSTEM 提示 */
	}
	else
	{
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::Calibration_Ended);
		/* LEGACY: TEXT("Calibration game finished.") */
	}
	ExitCalibrationGame();
}

void UTerminalWidget::ExitCalibrationGame()
{
	if (!Display)
	{
		return;
	}

	CurrentInputMode = ETerminalInputMode::Terminal;
	Display->ClearInputLine();
	Display->UpdateDisplay();

	AppendTerminalOutputBlock(CrankItNarrative::Terminal::Calibration_BackToTerminal);
	/* LEGACY: TEXT("Calibration game finished. Back to terminal.") */
	Display->StartDisplayingLines();

	if (CalibrationWidget)
	{
		CalibrationWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UTerminalWidget::TryStartBatteryHold(const FString& Command)
{
	if (!Display || !BatteryGatedCommands.Contains(Command))
	{
		return;
	}

	if (bBatteryHoldActive)
	{
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::Battery_TransferAlreadyActive);
		/* LEGACY: TEXT("Auxiliary transfer already in progress.") */
		Display->StartDisplayingLines();
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (!FBatteryPowerChecker::HasEnoughPower(World, RequiredChargedBatteryCount, MinBatteryChargeLevel))
	{
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::Battery_LowPowerError);
		/* LEGACY: ERROR: Low auxiliary power... */
		Display->StartDisplayingLines();
		return;
	}

	const float Now = World->GetTimeSeconds();
	bBatteryHoldActive = true;
	BatteryHoldEndTime = Now + BatteryHoldDurationSeconds;
	BatteryHoldLastDrainTime = Now;
	BatteryHoldLastProgressTime = Now;
	Display->ClearInputLine();
	Display->UpdateDisplay();

	AppendTerminalOutputBlock(CrankItNarrative::Terminal::Battery_TransferStarted);
	Display->AddPendingLine(FString::Printf(
		TEXT("Transfer remaining: %d seconds"),
		FMath::CeilToInt(BatteryHoldDurationSeconds)));
	Display->AddPendingLine(TEXT(""));
	/* LEGACY: Auxiliary power transfer initiated... */
	Display->StartDisplayingLines();

	World->GetTimerManager().SetTimer(
		BatteryHoldTimerHandle,
		this,
		&UTerminalWidget::OnBatteryHoldTick,
		BatteryHoldTickInterval,
		true
	);
}

void UTerminalWidget::OnBatteryHoldTick()
{
	if (!Display || !bBatteryHoldActive)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		EndBatteryHold(false);
		return;
	}

	const float Now = World->GetTimeSeconds();

	if (!FBatteryPowerChecker::HasEnoughPower(World, RequiredChargedBatteryCount, MinBatteryChargeLevel))
	{
		EndBatteryHold(false);
		return;
	}

	if (BatteryDrainInterval > KINDA_SMALL_NUMBER && Now - BatteryHoldLastDrainTime >= BatteryDrainInterval)
	{
		BatteryHoldLastDrainTime = Now;
		TArray<ABattery*> Batteries;
		FBatteryPowerChecker::GetBatteriesInTerminalSlots(World, MinBatteryChargeLevel, Batteries);
		for (ABattery* Battery : Batteries)
		{
			if (Battery)
			{
				Battery->SetChargeProgress(FMath::Max(0, Battery->ChargeProgress - BatteryDrainAmount));
			}
		}
		if (!FBatteryPowerChecker::HasEnoughPower(World, RequiredChargedBatteryCount, MinBatteryChargeLevel))
		{
			EndBatteryHold(false);
			return;
		}
	}

	if (Now - BatteryHoldLastProgressTime >= 10.f)
	{
		BatteryHoldLastProgressTime = Now;
		Display->AddPendingLine(FString::Printf(
			TEXT("Transfer remaining: %d seconds"),
			FMath::CeilToInt(FMath::Max(0.f, BatteryHoldEndTime - Now))));
		Display->StartDisplayingLines();
	}

	if (Now >= BatteryHoldEndTime)
	{
		EndBatteryHold(true);
	}
}

void UTerminalWidget::EndBatteryHold(bool bSuccess)
{
	if (!Display || !bBatteryHoldActive)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BatteryHoldTimerHandle);
	}

	bBatteryHoldActive = false;
	BatteryHoldEndTime = 0.f;

	if (bSuccess)
	{
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::Battery_TransferComplete);
		/* LEGACY: Auxiliary power transfer complete... */
	}
	else
	{
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::Battery_TransferInterrupted);
		/* LEGACY: transfer interrupted + low power error */
	}
	Display->StartDisplayingLines();
}

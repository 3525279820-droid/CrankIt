#include "TerminalWidget.h"

#include "TerminalMiniGameHost.h"
#include "TerminalBatteryHoldController.h"
#include "Components/TextBlock.h"
#include "PlayerCamera.h"
#include "CrankItNarrativeSubsystem.h"
#include "CrankItNarrativeIds.h"
#include "TerminalActionDispatcher.h"
#include "CrankItAudioService.h"

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
			if (Narrative->GetTerminalOutputLines(BlockId, Lines))
			{
				Display->AppendPendingLines(Lines);
			}
		}
	}
}

void UTerminalWidget::LoadCommandRouterFromAsset()
{
	FTerminalCommandRouterConfig Config;
	if (UWorld* World = GetWorld())
	{
		if (UCrankItNarrativeSubsystem* Narrative = World->GetSubsystem<UCrankItNarrativeSubsystem>())
		{
			if (Narrative->ApplyTerminalCommandRouterConfig(Config))
			{
				CommandRouter.LoadConfig(Config);
				return;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("TerminalWidget: 未加载 TerminalCommandData，请在 GameMode 上指定 DA_CrankItTerminalCommands。"));
}

void UTerminalWidget::StartBoardleGame()
{
	BoardleGame.StartSession();
}

void UTerminalWidget::TryStartBatteryHold(const FString& Command)
{
	if (BatteryHold)
	{
		BatteryHold->TryStart(Command);
	}
}

void UTerminalWidget::ApplyBoardleOutcomes(const TArray<FBoardleGuessOutcome>& Outcomes)
{
	if (!Display)
	{
		return;
	}

	bool bScheduleScreenClear = false;
	TFunction<void()> AfterScreenClear;

	for (const FBoardleGuessOutcome& Outcome : Outcomes)
	{
		switch (Outcome.Kind)
		{
		case FBoardleGuessOutcome::EKind::GuessSummary:
			Display->AddPendingLine(Outcome.Text);
			break;
		case FBoardleGuessOutcome::EKind::RoundWon:
			bScheduleScreenClear = true;
			AfterScreenClear = [this]()
			{
				AppendTerminalOutputBlock(CrankItNarrative::Terminal::Boordle_GuessCorrectFollowup);
				Display->StartDisplayingLines();
			};
			break;
		case FBoardleGuessOutcome::EKind::RoundLost:
			AppendTerminalOutputBlock(CrankItNarrative::Terminal::Boordle_OutOfGuessesPrefix);
			Display->AddPendingLine(Outcome.Text);
			Display->AddPendingLine(TEXT("Starting new round..."));
			if (Outcome.bScheduleReset)
			{
				bScheduleScreenClear = true;
				AfterScreenClear = [this]()
				{
					BoardleGame.ResetRound();
				};
			}
			break;
		default:
			break;
		}
	}

	Display->StartDisplayingLines();

	if (bScheduleScreenClear)
	{
		ScheduleBoardleScreenClear(MoveTemp(AfterScreenClear));
	}
}

void UTerminalWidget::ScheduleBoardleScreenClear(TFunction<void()> AfterClear)
{
	if (!Display)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(BoardleDeferredTimerHandle);

	World->GetTimerManager().SetTimer(
		BoardleDeferredTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this, AfterClear = MoveTemp(AfterClear)]() mutable
		{
			if (!Display)
			{
				return;
			}

			Display->ClearDisplayTimer();
			Display->ClearTerminal();
			Display->ClearPendingLines();
			Display->ClearInputLine();

			if (AfterClear)
			{
				AfterClear();
			}

			Display->UpdateDisplay();
		}),
		2.5f,
		false);
}

void UTerminalWidget::CommitBoardleGuess()
{
	if (!Display || Display->GetCurrentInputLine().Len() != BoardleGame.GetBinaryLength())
	{
		return;
	}

	const FString Guess = Display->GetCurrentInputLine();
	Display->AppendToCurrentText(Guess + TEXT("\n"));
	ApplyBoardleOutcomes(BoardleGame.SubmitGuess(Guess));
	Display->ClearInputLine();
	Display->UpdateDisplay();
}

void UTerminalWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Display = NewObject<UTerminalDisplayController>(this);
	Display->Initialize(this, TerminalText);
	Display->Interval = Interval;
	Display->ClearInputLine();

	SetIsFocusable(true);

	BoardleGame.Configure(6, 7);
	BoardleGame.ResetRound();

	LoadCommandRouterFromAsset();

	ActionDispatcher = NewObject<UTerminalActionDispatcher>(this);
	ActionDispatcher->Initialize(this, Display);

	MiniGameHost = NewObject<UTerminalMiniGameHost>(this);
	MiniGameHost->Initialize(this, Display, ClassificationGameWidget, CalibrationWidget);

	BatteryHold = NewObject<UTerminalBatteryHoldController>(this);
	BatteryHold->Initialize(this, Display);
	BatteryHold->Configure(
		RequiredChargedBatteryCount,
		MinBatteryChargeLevel,
		BatteryHoldDurationSeconds,
		BatteryHoldTickInterval,
		BatteryDrainInterval,
		BatteryDrainAmount);

	Display->ClearTerminal();
	AppendTerminalOutputBlock(CrankItNarrative::Terminal::ComputerScreen_Startup);
	Display->StartDisplayingLines();
}

void UTerminalWidget::PlayTerminalSound2D(USoundBase* Sound)
{
	if (UCrankItAudioService* Audio = UCrankItAudioService::Get(this))
	{
		Audio->Play2D(Sound);
	}
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
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BoardleDeferredTimerHandle);
	}
	if (Display)
	{
		Display->ClearDisplayTimer();
	}
	if (BatteryHold)
	{
		BatteryHold->Shutdown();
	}
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

void UTerminalWidget::RequestExitComputerView()
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

FReply UTerminalWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (BatteryHold && BatteryHold->IsActive())
	{
		if (Key == EKeys::Tab)
		{
			RequestExitComputerView();
		}
		return FReply::Handled();
	}

	if (MiniGameHost && MiniGameHost->RouteKey(Key))
	{
		CurrentInputMode = MiniGameHost->GetInputMode();
		return FReply::Handled();
	}

	CurrentInputMode = ETerminalInputMode::Terminal;

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
		PlayTerminalSound2D(BackspaceSound);
		Display->RemoveLastInputChar();
		Display->UpdateDisplay();
		return FReply::Handled();
	}
	if (Key == EKeys::Tab)
	{
		RequestExitComputerView();
		return FReply::Handled();
	}

	const uint32 CharCode = InKeyEvent.GetCharacter();
	if (CharCode != 0)
	{
		const TCHAR Char = static_cast<TCHAR>(CharCode);
		if (BoardleGame.IsActive())
		{
			if ((Char == TEXT('0') || Char == TEXT('1')) && Display->GetCurrentInputLine().Len() < BoardleGame.GetBinaryLength())
			{
				PlayTerminalSound2D(KeyInputSound);
				Display->AppendCharToInputLine(Char);
				CommitBoardleGuess();
				Display->UpdateDisplay();
			}
		}
		else
		{
			if (Char == TEXT(' '))
			{
				PlayTerminalSound2D(SpaceSound);
			}
			else
			{
				PlayTerminalSound2D(KeyInputSound);
			}
			Display->AppendCharToInputLine(Char);
			Display->UpdateDisplay();
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
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
	PlayTerminalSound2D(UnknownCommandSound);
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

	if (BatteryHold && BatteryHold->IsActive())
	{
		AppendTerminalOutputBlock(CrankItNarrative::Terminal::Battery_TransferInProgress);
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

	const int32 IndexBeforeSubmit = CommandRouter.GetNextCommandIndex();
	const FTerminalCommandSubmitResult Result = CommandRouter.Submit(SubmittedInput);
	UE_LOG(LogTemp, Display, TEXT("now index: %d"), IndexBeforeSubmit);

	switch (Result.Status)
	{
	case ETerminalSubmitStatus::PastEnd:
	case ETerminalSubmitStatus::WrongStage:
		HandleWrongCommand();
		break;

	case ETerminalSubmitStatus::AcceptedTextOnly:
	case ETerminalSubmitStatus::AcceptedAction:
	case ETerminalSubmitStatus::AcceptedUnknown:
		Display->ClearDisplayTimer();
		Display->ClearPendingLines();
		Display->ClearTerminal();
		bErrorShownForCurrentCommand = false;
		CurrentStageErrorText.Empty();
		CurrentStageWrongInputText.Empty();

		if (Result.Status == ETerminalSubmitStatus::AcceptedAction && ActionDispatcher)
		{
			ActionDispatcher->Execute(
				Result.ActionId,
				Result.MatchedCommand,
				&CommandRouter.GetCommandTextMap());
		}
		else if (Result.Status == ETerminalSubmitStatus::AcceptedTextOnly)
		{
			Display->AppendPendingLines(Result.TextLines);
			Display->StartDisplayingLines();
		}
		else
		{
			Display->AddPendingLine(FString::Printf(TEXT("Unknown command: %s"), *Result.MatchedCommand));
			PlayTerminalSound2D(UnknownCommandSound);
			Display->StartDisplayingLines();
		}
		break;

	default:
		HandleWrongCommand();
		break;
	}

	Display->UpdateDisplay();
}

void UTerminalWidget::EnterClassificationGame()
{
	if (MiniGameHost)
	{
		MiniGameHost->EnterClassificationGame();
		CurrentInputMode = MiniGameHost->GetInputMode();
	}
}

void UTerminalWidget::ExitClassificationGame()
{
	if (MiniGameHost)
	{
		MiniGameHost->ExitClassificationGame();
		CurrentInputMode = MiniGameHost->GetInputMode();
	}
}

void UTerminalWidget::EnterCalibrationGame(TFunction<void(bool)> OnComplete)
{
	if (MiniGameHost)
	{
		MiniGameHost->EnterCalibrationGame(MoveTemp(OnComplete));
		CurrentInputMode = MiniGameHost->GetInputMode();
	}
}

void UTerminalWidget::ExitCalibrationGame()
{
	if (MiniGameHost)
	{
		MiniGameHost->ExitCalibrationGame();
		CurrentInputMode = MiniGameHost->GetInputMode();
	}
}

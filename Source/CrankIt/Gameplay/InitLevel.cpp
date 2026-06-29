// Fill out your copyright notice in the Description page of Project Settings.


#include "InitLevel.h"
#include "Monster.h"
#include "MineConsole.h"
#include "PlayerCamera.h"
#include "ComputerScreenActor.h"
#include "KeyPromptWidgetBase.h"
#include "Tunnel.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "CrankItNarrativeSubsystem.h"
#include "CrankItNarrativeIds.h"

namespace
{
	// 通过叙事子系统按 TrackId 播放字幕
	void PlaySubtitleTrackById(
		UWorld* World,
		FName TrackId,
		TFunction<void()> OnComplete = TFunction<void()>())
	{
		if (UCrankItNarrativeSubsystem* Narrative = World ? World->GetSubsystem<UCrankItNarrativeSubsystem>() : nullptr)
		{
			Narrative->PlaySubtitleTrack(TrackId, MoveTemp(OnComplete));
		}
		else if (OnComplete)
		{
			OnComplete();
		}
	}

	ALevelSequenceActor* FindLevelSequenceActorByTag(UWorld* World, FName ActorTag)
	{
		if (!World || ActorTag.IsNone())
		{
			return nullptr;
		}

		for (TActorIterator<ALevelSequenceActor> It(World); It; ++It)
		{
			ALevelSequenceActor* const SeqActor = *It;
			if (SeqActor && SeqActor->ActorHasTag(ActorTag))
			{
				return SeqActor;
			}
		}
		return nullptr;
	}
}

AInitLevel::AInitLevel()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AInitLevel::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* World = GetWorld())
	{
		PC = World->GetFirstPlayerController();
		if (PC)
		{
			Cam = Cast<APlayerCamera>(PC->GetPawn());
		}
	}
	
	APlayerCamera::DisableAllInput(PC);

	// 将 GameMode 上配置的 Data Asset 注入叙事子系统
	if (UWorld* World = GetWorld())
	{
		if (UCrankItNarrativeSubsystem* Narrative = World->GetSubsystem<UCrankItNarrativeSubsystem>())
		{
			Narrative->SetNarrativeData(NarrativeData);
			Narrative->SetTerminalCommandData(TerminalCommandData);
		}
	}

	GetWorldTimerManager().SetTimer(
		DesendTimer,
		FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			ATunnel* Tunnel = Cast<ATunnel>(
				UGameplayStatics::GetActorOfClass(GetWorld(), ATunnel::StaticClass()));

			if (Tunnel)
			{
				Tunnel->bShouldMove = false;
				PlaySequence(IntroSequenceActorTag, false);
			}
		}),
		DesendTime,
		false
	);
	
}

void AInitLevel::OnTutorialClosed()
{
	if (TutorialWidget)
	{
		TutorialWidget->OnTutorialDismissed.RemoveDynamic(this, &AInitLevel::OnTutorialClosed);
		TutorialWidget = nullptr;
	}
	++CurrentTutorialIndex;
	if (!PC)
	{
		return;
	}
	APlayerCamera::ApplyExplorationInputMode(PC);

	PlaySubtitleTrackById(GetWorld(), CrankItNarrative::Subtitle::PostTutorial, [this]() {});
}

void AInitLevel::ShowKeyPrompt()
{
	if (!PC || !Cam || !Cam->KeyPromptWidgetClass)
	{
		return;
	}

	if (!Cam->KeyPromptWidget)
	{
		Cam->KeyPromptWidget = CreateWidget<UKeyPromptWidgetBase>(
			PC, Cam->KeyPromptWidgetClass);
	}

	UKeyPromptWidgetBase* const KeyPromptWidget = Cam->KeyPromptWidget;
	if (!KeyPromptWidget)
	{
		return;
	}

	KeyPromptWidget->SetupPrompt(
		NSLOCTEXT("Tutorial", "ADMove", "使用 A / D 键进行切换"));

	if (!KeyPromptWidget->IsInViewport())
	{
		KeyPromptWidget->AddToPlayerScreen(150);
	}

	APlayerCamera::ApplyExplorationInputMode(PC);
}

void AInitLevel::PlaySequence(FName SequenceTag, bool bLoop)
{
	UWorld* World = GetWorld();
	if (!World || !PC)
	{
		return;
	}
	if (SequenceTag.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("PlaySequence: SequenceTag is None."));
		return;
	}

	if (Cam)
	{
		Cam->bIsInCinematic = true;
		APlayerCamera::SetExplorationMappingContextEnabled(PC, false);
		PC->SetViewTarget(Cam);
	}
	PC->SetCinematicMode(
		true,
		true,
		false,
		true,
		true
	);

	ALevelSequenceActor* const LevelSequenceActor = FindLevelSequenceActorByTag(World, SequenceTag);
	if (!LevelSequenceActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlaySequence: LevelSequenceActor with tag '%s' not found."), *SequenceTag.ToString());
		return;
	}

	if (ULevelSequencePlayer* Player = LevelSequenceActor->GetSequencePlayer())
	{
		BoundIntroSequencePlayer = Player;
		bBoundSequenceLoops = bLoop;

		FMovieSceneSequencePlaybackSettings Settings = LevelSequenceActor->PlaybackSettings;
		Settings.LoopCount.Value = bLoop ? -1 : 0;
		Player->SetPlaybackSettings(Settings);

		Player->Stop();
		Player->OnFinished.RemoveDynamic(this, &AInitLevel::OnLevelSequenceFinished);
		Player->OnFinished.AddDynamic(this, &AInitLevel::OnLevelSequenceFinished);

		Player->PlayLooping(bLoop ? -1 : 0);
	}
}

void AInitLevel::SetFirstComputerScreenText()
{
	UWorld* World = GetWorld();

	ComputerScreen = Cast<AComputerScreenActor>(UGameplayStatics::GetActorOfClass(World, AComputerScreenActor::StaticClass()));
	if (ComputerScreen)
	{
		ComputerScreen->SetFirstPromptText();
	}
}

void AInitLevel::ShowSkipTutorial()
{
	if (Cam)
	{
		Cam->bIsInCinematic = true;
		APlayerCamera::SetExplorationMappingContextEnabled(PC, false);
	}
	PC->SetCinematicMode(
		true,
		true,
		false,
		true,
		true
	);
	if (Cam && Cam->SktWidgetClass)
	{
		Skt = CreateWidget<USkipTutorialWidget>(PC, Cam->SktWidgetClass);
		if (Skt)
		{
			Skt->AddToPlayerScreen(100);
			Skt->YesButtonClicked.AddDynamic(this, &AInitLevel::TutorialSkipped);
			Skt->NoButtonClicked.AddDynamic(this, &AInitLevel::TutorialNotSkipped);
			APlayerCamera::ApplyExplorationInputMode(PC);
		}
	}
}

void AInitLevel::ShowTutorial()
{
	if (!PC)
	{
		return;
	}

	if (CurrentTutorialIndex >= TutorialWidgetClasses.Num())
	{
		return;
	}

	const TSubclassOf<USDTutorialWidget> WidgetClass = TutorialWidgetClasses[CurrentTutorialIndex];
	if (!WidgetClass)
	{
		return;
	}

	if (!TutorialWidget)
	{
		TutorialWidget = CreateWidget<USDTutorialWidget>(PC, WidgetClass);
	}

	if (!TutorialWidget)
	{
		return;
	}

	TutorialWidget->OnTutorialDismissed.AddDynamic(this, &AInitLevel::OnTutorialClosed);
	TutorialWidget->AddToPlayerScreen();

	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(TutorialWidget->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PC->SetInputMode(Mode);
	PC->bShowMouseCursor = true;
}

void AInitLevel::PrepareLevel()
{
	ComputerScreen = Cast<AComputerScreenActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AComputerScreenActor::StaticClass()));
	if (ComputerScreen) ComputerScreen->bIsClickable = true;
	
	Monster = Cast<AMonster>(UGameplayStatics::GetActorOfClass(GetWorld(), AMonster::StaticClass()));
	if (Monster) Monster->bSpawnable = true;
}

void AInitLevel::StopIntroCutsceneAndReturnToGame()
{
	if (ULevelSequencePlayer* Player = BoundIntroSequencePlayer.Get())
	{
		Player->OnFinished.RemoveDynamic(this, &AInitLevel::OnLevelSequenceFinished);
		if (ALevelSequenceActor* LSA = Cast<ALevelSequenceActor>(Player->GetOuter()))
		{
			FMovieSceneSequencePlaybackSettings Settings = LSA->PlaybackSettings;
			Settings.LoopCount.Value = 0;
			Settings.bPauseAtEnd = false;
			Player->SetPlaybackSettings(Settings);
		}
		Player->Stop();
	}
	BoundIntroSequencePlayer.Reset();
	bBoundSequenceLoops = false;
	bBoundSequenceHoldAtEnd = false;
	APlayerCamera::EnableAllInput(PC);
	CleanupIntroUIAndRestoreGameplay();
}

void AInitLevel::OnLevelSequenceFinished()
{
	if (bBoundSequenceHoldAtEnd)
	{
		return;
	}

	if (bBoundSequenceLoops)
	{
		if (ULevelSequencePlayer* Player = BoundIntroSequencePlayer.Get())
		{
			if (Player->IsPlaying())
			{
				return;
			}
		}
	}

	APlayerCamera::EnableAllInput(PC);
	if (ULevelSequencePlayer* Player = BoundIntroSequencePlayer.Get())
	{
		Player->OnFinished.RemoveDynamic(this, &AInitLevel::OnLevelSequenceFinished);
	}
	BoundIntroSequencePlayer.Reset();
	bBoundSequenceLoops = false;
	bBoundSequenceHoldAtEnd = false;
	CleanupIntroUIAndRestoreGameplay();
}

void AInitLevel::CleanupIntroUIAndRestoreGameplay()
{
	if (Skt)
	{
		Skt->YesButtonClicked.RemoveDynamic(this, &AInitLevel::TutorialSkipped);
		Skt->NoButtonClicked.RemoveDynamic(this, &AInitLevel::TutorialNotSkipped);
		Skt->RemoveFromParent();
		Skt = nullptr;
	}

	if (Cam)
	{
		Cam->bIsInCinematic = false;
		if (PC)
		{
			APlayerCamera::SetExplorationMappingContextEnabled(PC, true);
			APlayerCamera::ApplyExplorationInputMode(PC);
		}
	}
	if (PC)
	{
		PC->SetCinematicMode(false, false, false, false, false);
	}
}

void AInitLevel::TutorialSkipped()
{
	if (Cam && Cam->MineConsole)
	{
		Cam->MineConsole->bSkipedTutorial = true;
	}
	StopIntroCutsceneAndReturnToGame();
	PlaySubtitleTrackById(GetWorld(), CrankItNarrative::Subtitle::TutorialSkipped, [this]() {});
}

void AInitLevel::TutorialNotSkipped()
{
	StopIntroCutsceneAndReturnToGame();
	APlayerCamera::DisableAllInput(PC);

	PlaySubtitleTrackById(
		GetWorld(),
		CrankItNarrative::Subtitle::TutorialNotSkipped_GordonIntro,
		[this]()
		{
			APlayerCamera::EnableAllInput(PC);
		});
}

void AInitLevel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);;
	if(!Cam)
	{
		return;
	}
	if(Cam->CurrentDirectionIndex == 3 && !bSkipTutorialFlowStarted)
	{
		APlayerCamera::DisableAllInput(PC);
		bSkipTutorialFlowStarted = true;

		PlaySubtitleTrackById(GetWorld(), CrankItNarrative::Subtitle::Intro_SkipTutorialPrompt, [this]()
		{
			bSkipTutorialFlowStarted = true;
			PlaySequence(TEXT("SkipTutorialSequencer"), true);
			ShowSkipTutorial();
		});
	}
	
}

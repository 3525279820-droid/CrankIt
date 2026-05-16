// Fill out your copyright notice in the Description page of Project Settings.


#include "InitLevel.h"
#include "PlayerCamera.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void AInitLevel::BeginPlay()
{
	Super::BeginPlay();

	// 无语音文件时：用世界时间轴跑几条测试字幕（上面已 AddToPlayerScreen；纯 C++ Widget 会自动建底栏 TextBlock）。
	
	if (UWorld* World = GetWorld())
	{
		SubtitleSys = World->GetSubsystem<USubtitleSubsystem>();
		if (SubtitleSys)
		{
			auto AddCue = [this](float Start, float End, const FString& Msg)
			{
				FCrankItSubtitleCue Cue;
				Cue.StartTimeSeconds = Start;
				Cue.EndTimeSeconds = End;
				Cue.Text = FText::FromString(Msg);
				TestCues.Add(Cue);
			};
			AddCue(0.f, 2.5f, FString(TEXT("【字幕测试】第一句（0~2.5 秒）")));
			AddCue(2.5f, 5.f, FString(TEXT("【字幕测试】第二句（2.5~5 秒）")));
			SubtitleSys->StartSubtitleTrackWithWorldTime(TestCues);

			float TrackEnd = TestCues.Last().EndTimeSeconds + 0.05f;
			UE_LOG(LogTemp, Display, TEXT("Track End Time is: %f"), TrackEnd)
			GetWorldTimerManager().SetTimer(
			SequencerTimer,
			this,
			&AInitLevel::ShowTutorial,
			TrackEnd,
			false
			);
		}
	}
}

void AInitLevel::OnTutorialClosed()
{
	if(TutorialWidget)
	{
		TutorialWidget->OnTutorialDismissed.RemoveDynamic(this, &AInitLevel::OnTutorialClosed);
		TutorialWidget = nullptr;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}
	APlayerCamera::ApplyExplorationInputMode(PC);

	if (!SubtitleSys)
	{
		SubtitleSys = World->GetSubsystem<USubtitleSubsystem>();
	}
	if (!SubtitleSys)
	{
		return;
	}

	TestCues.Reset();
	auto AddCue = [this](float Start, float End, const FString& Msg)
	{
		FCrankItSubtitleCue Cue;
		Cue.StartTimeSeconds = Start;
		Cue.EndTimeSeconds = End;
		Cue.Text = FText::FromString(Msg);
		TestCues.Add(Cue);
	};
	AddCue(0.f, 2.5f, FString(TEXT("【字幕测试】第三句（0~2.5 秒）")));
	AddCue(2.5f, 5.f, FString(TEXT("【字幕测试】第四句（2.5~5 秒）")));
	SubtitleSys->StartSubtitleTrackWithWorldTime(TestCues);

	float TrackEnd = TestCues.Last().EndTimeSeconds + 0.05f;
	UE_LOG(LogTemp, Display, TEXT("Track End Time is: %f"), TrackEnd)
	GetWorldTimerManager().SetTimer(
	SequencerTimer,
	this,
	&AInitLevel::PlaySequence,
	TrackEnd,
	false
	);

	
}

void AInitLevel::PlaySequence()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}
	APawn* Pawn = PC->GetPawn();
	APlayerCamera* Cam = Cast<APlayerCamera>(Pawn);
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
	ALevelSequenceActor* LevelSequenceActor = Cast<ALevelSequenceActor>(UGameplayStatics::GetActorOfClass(World, ALevelSequenceActor::StaticClass()));
	if (LevelSequenceActor)
	{
		if (ULevelSequencePlayer* Player = LevelSequenceActor->GetSequencePlayer())
		{
			BoundIntroSequencePlayer = Player;
			FMovieSceneSequencePlaybackSettings Settings = LevelSequenceActor->PlaybackSettings;
			Settings.LoopCount.Value = -1;
			Player->SetPlaybackSettings(Settings);

			Player->OnFinished.RemoveDynamic(this, &AInitLevel::OnLevelSequenceFinished);
			Player->OnFinished.AddDynamic(this, &AInitLevel::OnLevelSequenceFinished);
			Player->Play();

			if (Cam && Cam->SktWidgetClass)
			{
				Skt = CreateWidget<USkipTutorialWidget>(PC, Cam->SktWidgetClass);
				if (Skt)
				{
					Skt->AddToPlayerScreen(100);
					APlayerCamera::ApplyExplorationInputMode(PC);
				}
			}
		}
	}
}

void AInitLevel::ShowTutorial()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}

	APlayerCamera* Cam = Cast<APlayerCamera>(PC->GetPawn());
	if (!Cam || !Cam->TutorialWidgetClass)
	{
		return;
	}

	if (!TutorialWidget)
	{
		TutorialWidget = CreateWidget<USDTutorialWidget>(PC, Cam->TutorialWidgetClass);
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

void AInitLevel::StopIntroCutsceneAndReturnToGame()
{
	if (ULevelSequencePlayer* Player = BoundIntroSequencePlayer.Get())
	{
		Player->OnFinished.RemoveDynamic(this, &AInitLevel::OnLevelSequenceFinished);
		if (UWorld* World = GetWorld())
		{
			if (ALevelSequenceActor* LSA = Cast<ALevelSequenceActor>(UGameplayStatics::GetActorOfClass(World, ALevelSequenceActor::StaticClass())))
			{
				FMovieSceneSequencePlaybackSettings Settings = LSA->PlaybackSettings;
				Settings.LoopCount.Value = 0;
				Player->SetPlaybackSettings(Settings);
			}
		}
		Player->Stop();
	}
	BoundIntroSequencePlayer.Reset();
	CleanupIntroUIAndRestoreGameplay();
}

void AInitLevel::OnLevelSequenceFinished()
{
	if (ULevelSequencePlayer* Player = BoundIntroSequencePlayer.Get())
	{
		Player->OnFinished.RemoveDynamic(this, &AInitLevel::OnLevelSequenceFinished);
	}
	BoundIntroSequencePlayer.Reset();
	CleanupIntroUIAndRestoreGameplay();
}

void AInitLevel::CleanupIntroUIAndRestoreGameplay()
{
	if (Skt)
	{
		Skt->RemoveFromParent();
		Skt = nullptr;
	}

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (APawn* Pawn = PC->GetPawn())
			{
				if (APlayerCamera* Cam = Cast<APlayerCamera>(Pawn))
				{
					Cam->bIsInCinematic = false;
					APlayerCamera::SetExplorationMappingContextEnabled(PC, true);
					APlayerCamera::ApplyExplorationInputMode(PC);
				}
			}
			PC->SetCinematicMode(false, false, false, false, false);
		}
	}
}


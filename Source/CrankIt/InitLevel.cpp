// Fill out your copyright notice in the Description page of Project Settings.


#include "InitLevel.h"
#include "PlayerCamera.h"
#include "ComputerScreenActor.h"
#include "KeyPromptWidgetBase.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void AInitLevel::PlaySubtitleTrack(
	const TArray<FInitLevelSubtitleLine>& Lines,
	TFunction<void()> OnComplete)
{
	if (!SubtitleSys)
	{
		if (UWorld* World = GetWorld())
		{
			SubtitleSys = World->GetSubsystem<USubtitleSubsystem>();
		}
	}
	if (!SubtitleSys || Lines.Num() == 0)
	{
		if (OnComplete)
		{
			OnComplete();
		}
		return;
	}

	TestCues.Reset();
	for (const FInitLevelSubtitleLine& Line : Lines)
	{
		FCrankItSubtitleCue Cue;
		Cue.StartTimeSeconds = Line.StartTimeSeconds;
		Cue.EndTimeSeconds = Line.EndTimeSeconds;
		Cue.Text = FText::FromString(Line.Text);
		TestCues.Add(Cue);
	}
	SubtitleSys->StartSubtitleTrackWithWorldTime(TestCues);

	if (!OnComplete)
	{
		return;
	}

	const float TrackEnd = TestCues.Last().EndTimeSeconds + 0.05f;
	UE_LOG(LogTemp, Display, TEXT("Track End Time is: %f"), TrackEnd);
	GetWorldTimerManager().SetTimer(
		SequencerTimer,
		FTimerDelegate::CreateLambda([OnComplete = MoveTemp(OnComplete)]() { OnComplete(); }),
		TrackEnd,
		false);
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

	// 无语音文件时：用世界时间轴跑几条测试字幕（上面已 AddToPlayerScreen；纯 C++ Widget 会自动建底栏 TextBlock）。
	const TArray<FInitLevelSubtitleLine> IntroLines = {
		{0.f, 2.5f, TEXT("【字幕测试】第一句（0~2.5 秒）")},
		{2.5f, 5.f, TEXT("【字幕测试】第二句（2.5~5 秒）")},
	};
	PlaySubtitleTrack(IntroLines, [this]() {});
}

void AInitLevel::OnTutorialClosed()
{
	if(TutorialWidget)
	{
		TutorialWidget->OnTutorialDismissed.RemoveDynamic(this, &AInitLevel::OnTutorialClosed);
		TutorialWidget = nullptr;
	}
	if (!PC)
	{
		return;
	}
	APlayerCamera::ApplyExplorationInputMode(PC);

	const TArray<FInitLevelSubtitleLine> PostTutorialLines = {
		{0.f, 2.5f, TEXT("【字幕测试】教程测试文本2")},
	};
	PlaySubtitleTrack(PostTutorialLines, [this]() {});
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

void AInitLevel::PlaySequence()
{
	UWorld* World = GetWorld();
	if (!World || !PC)
	{
		return;
	}
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
					Skt->YesButtonClicked.AddDynamic(this, &AInitLevel::TutorialSkipped);
					Skt->NoButtonClicked.AddDynamic(this, &AInitLevel::TutorialNotSkipped);
					APlayerCamera::ApplyExplorationInputMode(PC);
				}
			}
		}
	}
	AComputerScreenActor* ComputerScreen = Cast<AComputerScreenActor>(UGameplayStatics::GetActorOfClass(World, AComputerScreenActor::StaticClass()));
	if (ComputerScreen)
	{
		ComputerScreen->SetFirstPromptText();
	}
}

void AInitLevel::ShowTutorial()
{
	if (!PC || !Cam || !Cam->TutorialWidgetClass)
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
	StopIntroCutsceneAndReturnToGame();
	PlaySubtitleTrack(
		TArray<FInitLevelSubtitleLine>(
			{
				{0.f, 2.5f, TEXT("【字幕测试】教程已跳过")}
		}),
		 [this]() { }
		);

}

void AInitLevel::TutorialNotSkipped()
{
	StopIntroCutsceneAndReturnToGame();
	PlaySubtitleTrack(
		TArray<FInitLevelSubtitleLine>(
			{
				{0.f, 2.5f, TEXT("【字幕测试】教程测试文本。")}
		}),
		 [this]() {ShowTutorial();}
		);
}
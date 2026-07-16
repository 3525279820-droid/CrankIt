#include "CrankItIntroFlowSubsystem.h"

#include "CrankItGameMode.h"
#include "Monster.h"
#include "PlayerCamera.h"
#include "ComputerScreenActor.h"
#include "KeyPromptWidgetBase.h"
#include "SkipTutorialWidget.h"
#include "SDTutorialWidget.h"
#include "Blueprint/UserWidget.h"
#include "Tunnel.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "LevelSequenceActor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "CrankItActorRegistry.h"
#include "CrankItGameplaySubsystem.h"
#include "CrankItNarrativeSubsystem.h"
#include "CrankItNarrativeIds.h"

const FName UCrankItIntroFlowSubsystem::SkipTutorialSequenceTag(TEXT("SkipTutorialSequencer"));

namespace
{
	// 按 Actor Tag 查找关卡中的 LevelSequenceActor
	ALevelSequenceActor* FindIntroLevelSequenceActorByTag(UWorld* World, FName ActorTag)
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

// 子系统销毁：解绑方向委托、清定时器与过场 OnFinished
void UCrankItIntroFlowSubsystem::Deinitialize()
{
	UnbindPlayerDirectionChanged();
	UnbindGameplayEvents();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DesendTimer);
	}

	if (ULevelSequencePlayer* Player = BoundIntroSequencePlayer.Get())
	{
		Player->OnFinished.RemoveDynamic(this, &UCrankItIntroFlowSubsystem::OnLevelSequenceFinished);
	}
	BoundIntroSequencePlayer.Reset();

	Super::Deinitialize();
}

// 由 ACrankItGameMode::BeginPlay 调用：缓存 GameMode、订阅朝向并启动开场定时器
void UCrankItIntroFlowSubsystem::StartIntroFlow(ACrankItGameMode* InOwnerGameMode)
{
	if (!InOwnerGameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode No found！"))
		return;
	}

	OwnerGameMode = InOwnerGameMode;
	UE_LOG(LogTemp, Warning, TEXT("StartIntroFlow！"))

	CachePlayerReferences();
	UnbindPlayerDirectionChanged();
	BindGameplayEvents();

	if (Cam)
	{
		Cam->OnDirectionChanged.AddUObject(this, &UCrankItIntroFlowSubsystem::HandlePlayerDirectionChanged);
		// 若开局已处于 West，与原先 GameMode Tick 首帧检测行为一致
		HandlePlayerDirectionChanged(Cam->CurrentDirectionIndex);
	}

	APlayerCamera::DisableAllInput(PC);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DesendTimer,
			this,
			&UCrankItIntroFlowSubsystem::OnDescendTimerFired,
			InOwnerGameMode->DesendTime,
			false
		);
	}
}

// 从 World 缓存 PlayerController 与 APlayerCamera
void UCrankItIntroFlowSubsystem::CachePlayerReferences()
{
	PC = nullptr;
	Cam = nullptr;

	if (UWorld* World = GetWorld())
	{
		PC = World->GetFirstPlayerController();
		if (PC)
		{
			Cam = Cast<APlayerCamera>(PC->GetPawn());
		}
	}
}

// 取消订阅 APlayerCamera::OnDirectionChanged
void UCrankItIntroFlowSubsystem::UnbindPlayerDirectionChanged()
{
	if (Cam)
	{
		Cam->OnDirectionChanged.RemoveAll(this);
	}
}

// 订阅 GameplaySubsystem::OnPostEMPTutorialFinished（StartIntroFlow 时调用）
void UCrankItIntroFlowSubsystem::BindGameplayEvents()
{
	if (UWorld* World = GetWorld())
	{
		if (UCrankItGameplaySubsystem* Gameplay = World->GetSubsystem<UCrankItGameplaySubsystem>())
		{
			Gameplay->OnPostEMPTutorialFinished.AddUObject(
				this, &UCrankItIntroFlowSubsystem::HandlePostEMPTutorialFinished);
		}
	}
}

// 子系统销毁时解除 GameplaySubsystem 订阅
void UCrankItIntroFlowSubsystem::UnbindGameplayEvents()
{
	if (UWorld* World = GetWorld())
	{
		if (UCrankItGameplaySubsystem* Gameplay = World->GetSubsystem<UCrankItGameplaySubsystem>())
		{
			Gameplay->OnPostEMPTutorialFinished.RemoveAll(this);
		}
	}
}

// EMP 教程字幕结束事件：解锁电脑屏幕点击与怪物生成
void UCrankItIntroFlowSubsystem::HandlePostEMPTutorialFinished()
{
	PrepareLevel();
}

// DesendTime 到期：停止隧道并播放 Intro 过场（Tag 来自 GameMode::IntroSequenceActorTag）
void UCrankItIntroFlowSubsystem::OnDescendTimerFired()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UCrankItActorRegistry* Registry = World->GetSubsystem<UCrankItActorRegistry>();
	// 经 ActorRegistry 取 ATunnel，停止下降后再播 Intro 过场
	ATunnel* Tunnel = Registry ? Registry->GetTunnel() : nullptr;
	if (!Tunnel)
	{
		return;
	}

	ACrankItGameMode* GM = OwnerGameMode.Get();
	if (!GM)
	{
		return;
	}

	Tunnel->bShouldMove = false;
	PlaySequence(GM->IntroSequenceActorTag, false);
}

// 玩家转向 West（Directions[3]）时触发 Skip 教程流程
void UCrankItIntroFlowSubsystem::HandlePlayerDirectionChanged(int32 NewDirectionIndex)
{
	if (NewDirectionIndex != SkipTutorialDirectionIndex || bSkipTutorialFlowStarted)
	{
		return;
	}

	StartSkipTutorialFlow();
}

// 播放 Skip 提示字幕 → 循环 Skip 过场 → 显示 Yes/No UI
void UCrankItIntroFlowSubsystem::StartSkipTutorialFlow()
{
	if (bSkipTutorialFlowStarted)
	{
		return;
	}

	bSkipTutorialFlowStarted = true;
	APlayerCamera::DisableAllInput(PC);

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (UCrankItNarrativeSubsystem* Narrative = World->GetSubsystem<UCrankItNarrativeSubsystem>())
	{
		// 字幕轨 ID 见 CrankItNarrativeIds.h
		Narrative->PlaySubtitleTrack(CrankItNarrative::Subtitle::Intro_SkipTutorialPrompt, [this]()
		{
			PlaySequence(SkipTutorialSequenceTag, true);
			ShowSkipTutorial();
		});
	}
}

// SD 教程 Widget 关闭：索引自增并播放 PostTutorial 字幕轨
void UCrankItIntroFlowSubsystem::OnTutorialClosed()
{
	if (TutorialWidget)
	{
		TutorialWidget->OnTutorialDismissed.RemoveDynamic(this, &UCrankItIntroFlowSubsystem::OnTutorialClosed);
		TutorialWidget = nullptr;
	}
	++CurrentTutorialIndex;
	if (!PC)
	{
		return;
	}
	APlayerCamera::ApplyExplorationInputMode(PC);
}

// 显示 A/D 移动键提示（Widget 类来自 APlayerCamera::KeyPromptWidgetClass）
void UCrankItIntroFlowSubsystem::ShowKeyPrompt()
{
	if (!PC || !Cam || !Cam->KeyPromptWidgetClass)
	{
		return;
	}

	if (!Cam->KeyPromptWidget)
	{
		Cam->KeyPromptWidget = CreateWidget<UKeyPromptWidgetBase>(PC, Cam->KeyPromptWidgetClass);
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

// 按 Tag 查找并播放 Level Sequence；bLoop 为 true 时不自动 OnFinished 恢复输入
void UCrankItIntroFlowSubsystem::PlaySequence(FName SequenceTag, bool bLoop)
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
		Cam->SetInCinematic(true);
		APlayerCamera::SetExplorationMappingContextEnabled(PC, false);
		PC->SetViewTarget(Cam);
	}
	PC->SetCinematicMode(true, true, false, true, true);

	ALevelSequenceActor* const LevelSequenceActor = FindIntroLevelSequenceActorByTag(World, SequenceTag);
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
		Player->OnFinished.RemoveDynamic(this, &UCrankItIntroFlowSubsystem::OnLevelSequenceFinished);
		Player->OnFinished.AddDynamic(this, &UCrankItIntroFlowSubsystem::OnLevelSequenceFinished);

		Player->PlayLooping(bLoop ? -1 : 0);
	}
}

// 设置电脑屏幕首段故障提示文案（经 ActorRegistry 取 AComputerScreenActor）
void UCrankItIntroFlowSubsystem::SetFirstComputerScreenText()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (UCrankItActorRegistry* Reg = World->GetSubsystem<UCrankItActorRegistry>())
	{
		ComputerScreen = Reg->GetComputerScreen();
	}
	if (ComputerScreen)
	{
		ComputerScreen->SetFirstPromptText();
	}
}

// 创建 SkipTutorial Widget 并绑定 Yes/No（类来自 APlayerCamera::SktWidgetClass）
void UCrankItIntroFlowSubsystem::ShowSkipTutorial()
{
	if (Cam)
	{
		Cam->SetInCinematic(true);
		APlayerCamera::SetExplorationMappingContextEnabled(PC, false);
	}
	if (PC)
	{
		PC->SetCinematicMode(true, true, false, true, true);
	}
	if (Cam && Cam->SktWidgetClass)
	{
		Skt = CreateWidget<USkipTutorialWidget>(PC, Cam->SktWidgetClass);
		if (Skt)
		{
			Skt->AddToPlayerScreen(100);
			Skt->YesButtonClicked.AddDynamic(this, &UCrankItIntroFlowSubsystem::TutorialSkipped);
			Skt->NoButtonClicked.AddDynamic(this, &UCrankItIntroFlowSubsystem::TutorialNotSkipped);
			APlayerCamera::ApplyExplorationInputMode(PC);
		}
	}
}

// 按 GameMode::TutorialWidgetClasses 顺序显示 SD 教程
void UCrankItIntroFlowSubsystem::ShowTutorial()
{
	ACrankItGameMode* GM = OwnerGameMode.Get();
	if (!GM || !PC)
	{
		return;
	}

	if (CurrentTutorialIndex >= GM->TutorialWidgetClasses.Num())
	{
		return;
	}

	const TSubclassOf<USDTutorialWidget> WidgetClass = GM->TutorialWidgetClasses[CurrentTutorialIndex];
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

	TutorialWidget->OnTutorialDismissed.AddDynamic(this, &UCrankItIntroFlowSubsystem::OnTutorialClosed);
	TutorialWidget->AddToPlayerScreen();

	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(TutorialWidget->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PC->SetInputMode(Mode);
	PC->bShowMouseCursor = true;
}

// 由 GameplaySubsystem::OnPostEMPTutorialFinished 等触发；经 ActorRegistry 解锁关卡元素
void UCrankItIntroFlowSubsystem::PrepareLevel()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (UCrankItActorRegistry* Reg = World->GetSubsystem<UCrankItActorRegistry>())
	{
		ComputerScreen = Reg->GetComputerScreen();
		Monster = Reg->GetMonster();
	}
	if (ComputerScreen)
	{
		ComputerScreen->SetInteractable(true);
	}
	if (Monster)
	{
		Monster->EnableSpawning();
	}
}

// 停止当前 Intro 过场并恢复探索输入（Skip / 继续教程共用）
void UCrankItIntroFlowSubsystem::StopIntroCutsceneAndReturnToGame()
{
	if (ULevelSequencePlayer* Player = BoundIntroSequencePlayer.Get())
	{
		Player->OnFinished.RemoveDynamic(this, &UCrankItIntroFlowSubsystem::OnLevelSequenceFinished);
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

// Level Sequence 自然结束：恢复输入并清理 Skip UI
void UCrankItIntroFlowSubsystem::OnLevelSequenceFinished()
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
		Player->OnFinished.RemoveDynamic(this, &UCrankItIntroFlowSubsystem::OnLevelSequenceFinished);
	}
	BoundIntroSequencePlayer.Reset();
	bBoundSequenceLoops = false;
	bBoundSequenceHoldAtEnd = false;
	CleanupIntroUIAndRestoreGameplay();
}

// 移除 Skip UI、关闭过场模式并恢复 EI 与探索输入模式
void UCrankItIntroFlowSubsystem::CleanupIntroUIAndRestoreGameplay()
{
	if (Skt)
	{
		Skt->YesButtonClicked.RemoveDynamic(this, &UCrankItIntroFlowSubsystem::TutorialSkipped);
		Skt->NoButtonClicked.RemoveDynamic(this, &UCrankItIntroFlowSubsystem::TutorialNotSkipped);
		Skt->RemoveFromParent();
		Skt = nullptr;
	}

	if (Cam)
	{
		Cam->SetInCinematic(false);
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

// Skip 教程选 Yes：经 GameplaySubsystem 写 MineConsole，再播放 TutorialSkipped 字幕轨
void UCrankItIntroFlowSubsystem::TutorialSkipped()
{
	if (UWorld* World = GetWorld())
	{
		if (UCrankItGameplaySubsystem* Gameplay = World->GetSubsystem<UCrankItGameplaySubsystem>())
		{
			Gameplay->SetTutorialSkipped(true);
		}
	}
	StopIntroCutsceneAndReturnToGame();


	if (UWorld* World = GetWorld())
	{
		APlayerCamera::DisableAllInput(PC);
	
		if (UCrankItNarrativeSubsystem* Narrative = World->GetSubsystem<UCrankItNarrativeSubsystem>())
		{
			Narrative->PlaySubtitleTrack(CrankItNarrative::Subtitle::TutorialSkipped, [this]()
			{
				APlayerCamera::EnableAllInput(PC);
				APlayerCamera* LiftCam = Cam;
				if (!LiftCam && PC)
				{
					LiftCam = Cast<APlayerCamera>(PC->GetPawn());
				}
				if (LiftCam)
				{
					LiftCam->StartSoundDetectorHoldLift();
				}
				PrepareLevel();
			});
		}
	}
}

// Skip 教程选 No：播放 Gordon 介绍字幕轨，结束后恢复输入
void UCrankItIntroFlowSubsystem::TutorialNotSkipped()
{
	StopIntroCutsceneAndReturnToGame();
	APlayerCamera::DisableAllInput(PC);

	if (UWorld* World = GetWorld())
	{
		if (UCrankItNarrativeSubsystem* Narrative = World->GetSubsystem<UCrankItNarrativeSubsystem>())
		{
			Narrative->PlaySubtitleTrack(
				CrankItNarrative::Subtitle::TutorialNotSkipped_GordonIntro,
				[this]()
				{
					APlayerCamera::EnableAllInput(PC);
					APlayerCamera* LiftCam = Cam;
					if (!LiftCam && PC)
					{
						LiftCam = Cast<APlayerCamera>(PC->GetPawn());
					}
					if (LiftCam)
					{
						LiftCam->StartSoundDetectorHoldLift();
					}
				});
		}
	}
}

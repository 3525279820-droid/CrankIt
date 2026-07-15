#include "CrankItGameplaySubsystem.h"

#include "CrankItActorRegistry.h"
#include "CrankItAudioService.h"
#include "CrankItGameMode.h"
#include "MineConsole.h"
#include "Monster.h"
#include "PlayerCamera.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

const FName UCrankItGameplaySubsystem::JumpScareSequenceTag(TEXT("JumpScare"));
const FName UCrankItGameplaySubsystem::GameOverMonsterLocationTag(TEXT("GameOverMonsterLocation"));

namespace
{
	// 按 Actor Tag 查找关卡中的 LevelSequenceActor（与 IntroFlow 同约定）
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

	// 按 Tag 取首个匹配 Actor（用于 GameOverMonsterLocation 占位）
	AActor* FindFirstActorWithTag(UWorld* World, FName ActorTag)
	{
		if (!World || ActorTag.IsNone())
		{
			return nullptr;
		}

		TArray<AActor*> Found;
		UGameplayStatics::GetAllActorsWithTag(World, ActorTag, Found);
		return Found.Num() > 0 ? Found[0] : nullptr;
	}
}

// 子系统销毁时解绑 JumpScare OnFinished，避免悬空回调
void UCrankItGameplaySubsystem::Deinitialize()
{
	if (ULevelSequencePlayer* Player = BoundJumpScarePlayer.Get())
	{
		Player->OnFinished.RemoveDynamic(this, &UCrankItGameplaySubsystem::OnJumpScareSequenceFinished);
	}
	BoundJumpScarePlayer.Reset();
	bJumpScarePlaying = false;
	Super::Deinitialize();
}

// 广播 OnPostEMPTutorialFinished，供 IntroFlow::PrepareLevel 等订阅者响应
void UCrankItGameplaySubsystem::NotifyPostEMPTutorialFinished()
{
	OnPostEMPTutorialFinished.Broadcast();
}

// 经 ActorRegistry 找到 AMineConsole 并标记教程已跳过
void UCrankItGameplaySubsystem::SetTutorialSkipped(bool bSkipped)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (UCrankItActorRegistry* Registry = World->GetSubsystem<UCrankItActorRegistry>())
	{
		if (AMineConsole* Console = Registry->GetMineConsole())
		{
			Console->SetTutorialSkipped(bSkipped);
		}
	}
}

// 缓存 PlayerController 与 APlayerCamera，供过场锁输入 / SetViewTarget
void UCrankItGameplaySubsystem::CachePlayerReferences()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	PC = World->GetFirstPlayerController();
	Cam = PC ? Cast<APlayerCamera>(PC->GetPawn()) : nullptr;
	if (!Cam)
	{
		Cam = Cast<APlayerCamera>(UGameplayStatics::GetPlayerPawn(World, 0));
	}
}

// 锁探索输入并以 PlayerCamera 为 ViewTarget（Sequence 绑定该 Pawn 镜头）
void UCrankItGameplaySubsystem::LockPlayerForCinematic()
{
	CachePlayerReferences();
	if (!PC)
	{
		return;
	}

	APlayerCamera::DisableAllInput(PC);
	if (Cam)
	{
		Cam->SetInCinematic(true);
		APlayerCamera::SetExplorationMappingContextEnabled(PC, false);
		PC->SetViewTarget(Cam);
	}
	PC->SetCinematicMode(true, true, false, true, true);
}

// 播放关卡中 Tag=JumpScare 的 LevelSequenceActor；成功返回 true
bool UCrankItGameplaySubsystem::PlayJumpScareSequence()
{
	UWorld* World = GetWorld();
	if (!World || !PC)
	{
		return false;
	}

	ALevelSequenceActor* const LevelSequenceActor = FindLevelSequenceActorByTag(World, JumpScareSequenceTag);
	if (!LevelSequenceActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayJumpScareSequence: LevelSequenceActor with tag '%s' not found."),
			*JumpScareSequenceTag.ToString());
		return false;
	}

	ULevelSequencePlayer* const Player = LevelSequenceActor->GetSequencePlayer();
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayJumpScareSequence: SequencePlayer is null on '%s'."),
			*GetNameSafe(LevelSequenceActor));
		return false;
	}

	if (ULevelSequencePlayer* Prev = BoundJumpScarePlayer.Get())
	{
		Prev->OnFinished.RemoveDynamic(this, &UCrankItGameplaySubsystem::OnJumpScareSequenceFinished);
	}

	BoundJumpScarePlayer = Player;

	FMovieSceneSequencePlaybackSettings Settings = LevelSequenceActor->PlaybackSettings;
	Settings.LoopCount.Value = 0;
	Player->SetPlaybackSettings(Settings);

	Player->Stop();
	Player->OnFinished.RemoveDynamic(this, &UCrankItGameplaySubsystem::OnJumpScareSequenceFinished);
	Player->OnFinished.AddDynamic(this, &UCrankItGameplaySubsystem::OnJumpScareSequenceFinished);
	Player->Play();
	return true;
}

// 由 AMonster 触达 MaxAdvanceCount 时调用：摆位 → 锁输入 → 播 JumpScare
void UCrankItGameplaySubsystem::TriggerGameOverJumpScare(AMonster* Monster)
{
	if (bJumpScarePlaying)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	AMonster* TargetMonster = Monster;
	if (!TargetMonster)
	{
		if (UCrankItActorRegistry* Registry = World->GetSubsystem<UCrankItActorRegistry>())
		{
			TargetMonster = Registry->GetMonster();
		}
	}
	if (!TargetMonster)
	{
		UE_LOG(LogTemp, Warning, TEXT("TriggerGameOverJumpScare: Monster not found."));
		return;
	}

	AActor* const LocationActor = FindFirstActorWithTag(World, GameOverMonsterLocationTag);
	if (!LocationActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("TriggerGameOverJumpScare: Actor with tag '%s' not found."),
			*GameOverMonsterLocationTag.ToString());
		return;
	}

	bJumpScarePlaying = true;

	TargetMonster->BeginGameOverJumpScare(LocationActor->GetActorTransform());
	LockPlayerForCinematic();
	PlayJumpScareSound();

	if (!PlayJumpScareSequence())
	{
		// Sequence 缺失时仍走结束回调，避免卡在半状态（输入已锁）
		OnJumpScareSequenceFinished();
	}
}

// 经 AudioService 播 JumpScareSound；未在 GameMode 配置则跳过
void UCrankItGameplaySubsystem::PlayJumpScareSound()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const ACrankItGameMode* const GM = World->GetAuthGameMode<ACrankItGameMode>();
	if (!GM || !GM->JumpScareSound)
	{
		return;
	}

	if (UCrankItAudioService* Audio = UCrankItAudioService::Get(this))
	{
		Audio->Play2D(GM->JumpScareSound);
	}
}

// JumpScare 播完：解绑 Player，广播 OnJumpScareFinished（不恢复输入）
void UCrankItGameplaySubsystem::OnJumpScareSequenceFinished()
{
	if (ULevelSequencePlayer* Player = BoundJumpScarePlayer.Get())
	{
		Player->OnFinished.RemoveDynamic(this, &UCrankItGameplaySubsystem::OnJumpScareSequenceFinished);
	}
	BoundJumpScarePlayer.Reset();
	bJumpScarePlaying = false;

	OnJumpScareFinished.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("JumpScare finished. Game Over."));
}

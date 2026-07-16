#pragma once

// 关卡玩法事件总线：MineConsole / IntroFlow 解耦，以及失败 JumpScare 播控

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrankItGameplaySubsystem.generated.h"

class AMonster;
class APlayerController;
class APlayerCamera;
class ULevelSequencePlayer;

// EMP 教程字幕轨全部播完后广播；IntroFlow 订阅后解锁关卡
DECLARE_MULTICAST_DELEGATE(FOnPostEMPTutorialFinished);

// JumpScare Sequence 播放结束；此后保持锁输入，供 Game Over UI 等衔接
DECLARE_MULTICAST_DELEGATE(FOnJumpScareFinished);

UCLASS()
class CRANKIT_API UCrankItGameplaySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	FOnPostEMPTutorialFinished OnPostEMPTutorialFinished;
	FOnJumpScareFinished OnJumpScareFinished;

	// 由 AMineConsole 在 EMP 教程字幕 OnComplete 回调末尾调用
	void NotifyPostEMPTutorialFinished();

	// Skip 教程选 Yes 等路径统一经此写入 MineConsole（内部经 ActorRegistry 查找）
	void SetTutorialSkipped(bool bSkipped);

	// 北向入口门开到位后调用：经 ActorRegistry 允许 Monster 在 North 生成
	void NotifyNorthEntryDoorOpened();

	// 失败跳杀：怪物移到 Tag=GameOverMonsterLocation，以 PlayerCamera 播 Tag=JumpScare 的 Sequence；Monster 为空时经 ActorRegistry 查找
	void TriggerGameOverJumpScare(AMonster* Monster = nullptr);

	UFUNCTION(BlueprintPure, Category = "Gameplay|GameOver")
	bool IsJumpScarePlaying() const { return bJumpScarePlaying; }

private:
	UFUNCTION()
	void OnJumpScareSequenceFinished();

	void CachePlayerReferences();
	void LockPlayerForCinematic();
	bool PlayJumpScareSequence();
	// 播放 GameMode::JumpScareSound（2D，与 Sequence 同时触发）
	void PlayJumpScareSound();

	UPROPERTY()
	TObjectPtr<APlayerController> PC = nullptr;

	UPROPERTY()
	TObjectPtr<APlayerCamera> Cam = nullptr;

	TWeakObjectPtr<ULevelSequencePlayer> BoundJumpScarePlayer;

	bool bJumpScarePlaying = false;

	// 关卡中 LevelSequenceActor 的 Actor Tag
	static const FName JumpScareSequenceTag;

	// 关卡中不可视占位 Actor 的 Tag；失败时怪物移到此处
	static const FName GameOverMonsterLocationTag;
};

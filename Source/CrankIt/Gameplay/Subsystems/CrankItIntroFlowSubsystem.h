#pragma once

// 开场 / 教程流程子系统：过场 Sequence、Skip 教程 UI、方向触发字幕等。
// ACrankItGameMode 只保留 EditAnywhere 配置与本子系统的启动入口，具体逻辑在此实现。

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrankItIntroFlowSubsystem.generated.h"

class ACrankItGameMode;
class APlayerController;
class APlayerCamera;
class AComputerScreenActor;
class AMonster;
class USkipTutorialWidget;
class USDTutorialWidget;
class ULevelSequencePlayer;

UCLASS()
class CRANKIT_API UCrankItIntroFlowSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	// BeginPlay 时由 ACrankItGameMode 调用：缓存 GameMode 配置、订阅方向变化并启动开场定时器
	void StartIntroFlow(ACrankItGameMode* OwnerGameMode);

	// 解锁怪物生成与电脑屏幕点击（通常由 HandlePostEMPTutorialFinished 调用）
	void PrepareLevel();

	void ShowKeyPrompt();
	void ShowTutorial();
	void SetFirstComputerScreenText();
	void StopIntroCutsceneAndReturnToGame();
	void ShowSkipTutorial();

	/** 按关卡 Level Sequence Actor Tag 播放过场；bLoop 为 true 时无限循环。 */
	void PlaySequence(FName SequenceTag, bool bLoop);

	UFUNCTION()
	void OnTutorialClosed();

	UFUNCTION()
	void TutorialSkipped();

	UFUNCTION()
	void TutorialNotSkipped();

private:
	UFUNCTION()
	void OnLevelSequenceFinished();

	void CleanupIntroUIAndRestoreGameplay();
	void HandlePlayerDirectionChanged(int32 NewDirectionIndex);
	void StartSkipTutorialFlow();
	void OnDescendTimerFired();
	void UnbindPlayerDirectionChanged();
	// 订阅 UCrankItGameplaySubsystem::OnPostEMPTutorialFinished
	void BindGameplayEvents();
	// Deinitialize 时解除 GameplaySubsystem 订阅
	void UnbindGameplayEvents();
	// OnPostEMPTutorialFinished 回调，转发 PrepareLevel
	void HandlePostEMPTutorialFinished();
	void CachePlayerReferences();

	TWeakObjectPtr<ACrankItGameMode> OwnerGameMode;

	TWeakObjectPtr<ULevelSequencePlayer> BoundIntroSequencePlayer;

	/** 当前绑定的过场是否为无限循环；循环时不应响应 OnFinished 恢复玩家相机。 */
	bool bBoundSequenceLoops = false;

	/** 播完后暂停在最后一帧，等待外部（如 Skip UI）再 Stop。 */
	bool bBoundSequenceHoldAtEnd = false;

	UPROPERTY()
	AComputerScreenActor* ComputerScreen = nullptr;

	UPROPERTY()
	APlayerController* PC = nullptr;

	UPROPERTY()
	APlayerCamera* Cam = nullptr;

	UPROPERTY()
	USkipTutorialWidget* Skt = nullptr;

	bool bSkipTutorialFlowStarted = false;

	UPROPERTY()
	USDTutorialWidget* TutorialWidget = nullptr;

	int32 CurrentTutorialIndex = 0;

	FTimerHandle DesendTimer;

	AMonster* Monster = nullptr;

	/** Skip 教程过场 Actor Tag（与关卡中 LevelSequenceActor 标签一致）。 */
	static const FName SkipTutorialSequenceTag;

	/** 触发 Skip 教程字幕与 UI 的朝向索引（Directions 中 West = 3）。 */
	static constexpr int32 SkipTutorialDirectionIndex = 3;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SubtitleSubsystem.h"
#include "LevelSequenceActor.h"
#include "SDTutorialWidget.h"
#include "SkipTutorialWidget.h"
#include "Tunnel.h"
#include "ComputerScreenActor.h"
#include "InitLevel.generated.h"

class ULevelSequencePlayer;
class APlayerController;
class APlayerCamera;
class AMonster;
class UCrankItNarrativeData;
class UCrankItTerminalCommandData;

/**
 * 
 */
UCLASS()
class CRANKIT_API AInitLevel : public AGameModeBase
{
	GENERATED_BODY()

public:
	AInitLevel();

private:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnLevelSequenceFinished();

	void CleanupIntroUIAndRestoreGameplay();

	TWeakObjectPtr<ULevelSequencePlayer> BoundIntroSequencePlayer;

	/** 当前绑定的过场是否为无限循环；循环时不应响应 OnFinished 恢复玩家相机。 */
	bool bBoundSequenceLoops = false;

	UPROPERTY()
	AComputerScreenActor* ComputerScreen = nullptr;

	/** 播完后暂停在最后一帧，等待外部（如 Skip UI）再 Stop。 */
	bool bBoundSequenceHoldAtEnd = false;

	UPROPERTY()
	APlayerController* PC = nullptr;

	UPROPERTY()
	APlayerCamera* Cam = nullptr;

	UPROPERTY()
	USkipTutorialWidget* Skt = nullptr;

	bool bSkipTutorialFlowStarted = false;

	virtual void Tick(float DeltaTime) override;

	
public:
	UFUNCTION()
	void OnTutorialClosed();
	void ShowKeyPrompt();

	UFUNCTION()
	void TutorialSkipped();

	UFUNCTION()
	void TutorialNotSkipped();
	
	/** 按关卡中 Level Sequence Actor 的 Tag 播放过场；bLoop 为 true 时无限循环；bHoldAtEndUntilStopped 为 true 时播完停在末帧且不自动恢复输入。 */
	void PlaySequence(FName SequenceTag, bool bLoop);
	
	void SetFirstComputerScreenText();

	void ShowTutorial();

	void ShowSkipTutorial();

	void PrepareLevel();

	/** 跳过教程 / 继续教程：停过场、关 UI、恢复操作与 EI（当前两者行为一致，后续可再分支）。 */
	void StopIntroCutsceneAndReturnToGame();

	/** 教程 Widget 类列表（须继承 USDTutorialWidget）；按顺序显示，关闭后索引自增。 */
	UPROPERTY(EditAnywhere, Category="Tutorial")
	TArray<TSubclassOf<USDTutorialWidget>> TutorialWidgetClasses;

	UPROPERTY()
	USDTutorialWidget* TutorialWidget = nullptr;

	int32 CurrentTutorialIndex = 0;

	FTimerHandle DesendTimer;
	FTimerHandle PauseAtEndTimer;
	UPROPERTY(EditAnywhere)
	float DesendTime = 10.f;

	/** BeginPlay 定时器触发的默认过场 Actor Tag。 */
	UPROPERTY(EditAnywhere, Category = "Cinematic")
	FName IntroSequenceActorTag;

	/** 字幕剧本 Primary Data Asset；TrackId 见 CrankItNarrativeIds.h */
	UPROPERTY(EditDefaultsOnly, Category = "Narrative")
	TObjectPtr<UCrankItNarrativeData> NarrativeData;

	/** 终端命令与控制台输出 Primary Data Asset */
	UPROPERTY(EditDefaultsOnly, Category = "Narrative")
	TObjectPtr<UCrankItTerminalCommandData> TerminalCommandData;

	AMonster* Monster = nullptr;

};

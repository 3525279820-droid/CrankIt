// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "InitLevel.generated.h"

class UCrankItNarrativeData;
class UCrankItTerminalCommandData;
class USDTutorialWidget;
class UCrankItIntroFlowSubsystem;

/**
 * 初始关卡 GameMode：仅保留编辑器可配项，并启动 UCrankItIntroFlowSubsystem。
 * 开场 / 教程 / 过场逻辑见 Gameplay/CrankItIntroFlowSubsystem。
 */
UCLASS()
class CRANKIT_API AInitLevel : public AGameModeBase
{
	GENERATED_BODY()

public:
	AInitLevel();

private:
	virtual void BeginPlay() override;

public:
	UFUNCTION()
	void OnTutorialClosed();

	void ShowKeyPrompt();

	UFUNCTION()
	void TutorialSkipped();

	UFUNCTION()
	void TutorialNotSkipped();

	/** 按关卡中 Level Sequence Actor 的 Tag 播放过场（转发至 IntroFlowSubsystem）。 */
	void PlaySequence(FName SequenceTag, bool bLoop);

	void SetFirstComputerScreenText();
	void ShowTutorial();
	void ShowSkipTutorial();

	/** 解锁怪物与电脑屏幕（转发至 IntroFlowSubsystem；EMP 教程结束经 GameplaySubsystem 事件触发）。 */
	void PrepareLevel();

	void StopIntroCutsceneAndReturnToGame();

	/** 教程 Widget 类列表（须继承 USDTutorialWidget）；按顺序显示，关闭后索引自增。 */
	UPROPERTY(EditAnywhere, Category="Tutorial")
	TArray<TSubclassOf<USDTutorialWidget>> TutorialWidgetClasses;

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

private:
	UCrankItIntroFlowSubsystem* GetIntroFlow() const;
};

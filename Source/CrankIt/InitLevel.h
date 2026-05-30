// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SubtitleSubsystem.h"
#include "LevelSequenceActor.h"
#include "SDTutorialWidget.h"
#include "SkipTutorialWidget.h"
#include "Tunnel.h"
#include "InitLevel.generated.h"

class ULevelSequencePlayer;
class APlayerController;
class APlayerCamera;

/** 单条字幕输入（流程内使用，会转为 FCrankItSubtitleCue）。 */
struct FInitLevelSubtitleLine
{
	float StartTimeSeconds = 0.f;
	float EndTimeSeconds = 0.f;
	FString Text;
};

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

	FTimerHandle SequencerTimer;

	TArray<FCrankItSubtitleCue> TestCues;

	USubtitleSubsystem* SubtitleSys = nullptr;

	/** 按世界时间播放字幕轨；全部结束后执行 OnComplete（默认可为空）。 */
	void PlaySubtitleTrack(
		const TArray<FInitLevelSubtitleLine>& Lines,
		TFunction<void()> OnComplete = TFunction<void()>());

	UFUNCTION()
	void OnLevelSequenceFinished();

	void CleanupIntroUIAndRestoreGameplay();

	void DisableAllInput();
	void EnableAllInput();

	TWeakObjectPtr<ULevelSequencePlayer> BoundIntroSequencePlayer;

	UPROPERTY()
	APlayerController* PC = nullptr;

	UPROPERTY()
	APlayerCamera* Cam = nullptr;

	UPROPERTY()
	USkipTutorialWidget* Skt = nullptr;

	UPROPERTY()
	USDTutorialWidget* TutorialWidget = nullptr;

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
	
	/** 按关卡中 Level Sequence Actor 的 Tag 播放过场；bLoop 为 true 时无限循环。 */
	void PlaySequence(FName SequenceTag, bool bLoop);
	
	void SetFirstComputerScreenText();

	void ShowTutorial();

	void ShowSkipTutorial();


	/** 跳过教程 / 继续教程：停过场、关 UI、恢复操作与 EI（当前两者行为一致，后续可再分支）。 */
	void StopIntroCutsceneAndReturnToGame();

	FTimerHandle DesendTimer;

	UPROPERTY(EditAnywhere)
	float DesendTime = 10.f;

	/** BeginPlay 定时器触发的默认过场 Actor Tag。 */
	UPROPERTY(EditAnywhere, Category = "Cinematic")
	FName IntroSequenceActorTag;

};

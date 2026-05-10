// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SubtitleSubsystem.h"
#include "LevelSequenceActor.h"
#include "SkipTutorialWidget.h"
#include "InitLevel.generated.h"

class ULevelSequencePlayer;

/**
 * 
 */
UCLASS()
class CRANKIT_API AInitLevel : public AGameModeBase
{
	GENERATED_BODY()
private:
	virtual void BeginPlay() override;

	FTimerHandle SequencerTimer;

	/** OnFinished 为动态委托，须 UFUNCTION + AddDynamic。 */
	UFUNCTION()
	void OnLevelSequenceFinished();

	void CleanupIntroUIAndRestoreGameplay();

	TWeakObjectPtr<ULevelSequencePlayer> BoundIntroSequencePlayer;

	UPROPERTY()
	USkipTutorialWidget* Skt = nullptr;

public:
	void PlaySequence();

	/** 跳过教程 / 继续教程：停过场、关 UI、恢复操作与 EI（当前两者行为一致，后续可再分支）。 */
	void StopIntroCutsceneAndReturnToGame();

};

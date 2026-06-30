// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ComputerScreenActor.h"
#include "KeyPromptWidgetBase.h"
#include "SubtitleWidget.h"
#include "SDTutorialWidget.h"
#include "LevelSequenceActor.h"
#include "SkipTutorialWidget.h"

#include "PlayerCamera.generated.h"

class AMineConsole;
class ABattery;

/** 玩家朝向索引变化时广播；IntroFlow 等可订阅，避免 GameMode Tick 轮询。 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerDirectionChanged, int32);

UCLASS()
class CRANKIT_API APlayerCamera : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerCamera();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void InteractInput(const FInputActionValue& InputActionValue);
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void TurnInput(const FInputActionValue& value);

	void ExitScreenInput(const FInputActionValue& value);

	void PickBattery();
	
	static void ApplyExplorationInputMode(APlayerController* PC);

	static void SetExplorationMappingContextEnabled(APlayerController* PC, bool bEnabled);

	/** 教程/字幕期间：关闭探索操作（过场模式、无鼠标点击）。 */
	static void DisableAllInput(APlayerController* PC);

	/** 教程/字幕结束后：恢复探索操作。 */
	static void EnableAllInput(APlayerController* PC);
	
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArmComp;
	
	/** 玩家视角；ASoundDetectorActor 以其世界位置与朝前作为声音锥检测原点/正向（非探测器模型） */
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* SoundDetectorHoldPoint;

	/** 相对基准位置向上抬升的最大距离（厘米） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SoundDetector|Hold")
	float SoundDetectorHoldLiftDistance = 20.f;

	/** 抬升速度（厘米/秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SoundDetector|Hold")
	float SoundDetectorHoldLiftSpeed = 80.f;

	/** 字幕等事件结束后调用，在 Tick 中平滑抬升 SoundDetectorHoldPoint */
	UFUNCTION(BlueprintCallable, Category = "SoundDetector|Hold")
	void StartSoundDetectorHoldLift();

	UPROPERTY(VisibleAnywhere)
	USceneComponent* BatteryHoldPoint;

	/** 拾起时线性插值：Alpha 每秒增加量（1 ≈ 约 1 秒从起点到当前挂点） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battery|Pickup")
	float BatteryPickupLerpSpeed = 2.f;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* TurnAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* IntereactAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ExitScreen;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Subtitle")
	TSubclassOf<USubtitleWidget> SubtitleWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Tutorial")
	TSubclassOf<USkipTutorialWidget> SktWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Tutorial")
	TSubclassOf<UKeyPromptWidgetBase> KeyPromptWidgetClass;

	UPROPERTY()
	UKeyPromptWidgetBase* KeyPromptWidget = nullptr;
	
	UPROPERTY(EditAnywhere, Category= "Cinematic")
	TObjectPtr<ULevelSequence> CinematicSequence;
	
	UPROPERTY()
	AActor* OriginalViewTarget;
	
	FRotator DeltaRotation = FRotator::ZeroRotator;

	APlayerController* PlayerController;

	APlayerController* PlayerControllerRef;

	AMineConsole* MineConsole;

	ABattery* TargetBattery;

	ABattery* HoldBattery;

	FTimerHandle SequenceTimer;

	bool bIsInCinematic = false;

	int32 CurrentDirectionIndex = 2;

	UPROPERTY(EditAnywhere, Category="PlayerCamera")
	TArray<FString> Directions = {"North", "East", "South", "West"};

	void UpdateCurrentDirection(bool bIsLeft);

	/** 朝向索引变化；IntroFlow 订阅后触发 Skip 教程流程。 */
	FOnPlayerDirectionChanged OnDirectionChanged;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
	USubtitleWidget* SubtitlesWidget = nullptr;

	/** BeginPlay 时从组件读取，作为抬升起点 */
	FVector SoundDetectorHoldBaseRelativeLocation = FVector::ZeroVector;
	float SoundDetectorHoldCurrentLift = 0.f;
	bool bLiftSoundDetectorHoldPoint = false;

	void UpdateSoundDetectorHoldLift(float DeltaTime);

	/** 交互后正平滑移向 BatteryHoldPoint，到位后写入 HoldBattery */
	ABattery* BatteryMovingToHold = nullptr;

	FVector BatteryPickupStartLoc = FVector::ZeroVector;
	FQuat BatteryPickupStartQuat = FQuat::Identity;
	float BatteryPickupMoveAlpha = 0.f;

	void UpdateBatteryPickupMotion(float DeltaTime);
};

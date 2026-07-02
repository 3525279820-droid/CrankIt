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
class UPlayerInteractionComponent;
class UBatteryHoldComponent;

// 玩家朝向索引变化时广播；IntroFlow 等可订阅，避免 GameMode Tick 轮询
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

	// 以下四个静态方法转发至 UCrankItInputModeService，保留旧调用点兼容
	static void ApplyExplorationInputMode(APlayerController* PC);
	static void SetExplorationMappingContextEnabled(APlayerController* PC, bool bEnabled);
	static void DisableAllInput(APlayerController* PC);
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

	// 鼠标悬停检测（ChargeHandle / Battery 拾取目标）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Interaction")
	UPlayerInteractionComponent* InteractionComponent = nullptr;

	// 电池拾取、持有与挂点跟随
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Battery")
	UBatteryHoldComponent* BatteryHoldComponent = nullptr;

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

	// IntroFlow / LightTrigger 等仍经此读取；实际由 InteractionComponent 缓存
	AMineConsole* MineConsole = nullptr;

	bool bIsInCinematic = false;

	int32 CurrentDirectionIndex = 2;

	UPROPERTY(EditAnywhere, Category="PlayerCamera")
	TArray<FString> Directions = {"North", "East", "South", "West"};

	void UpdateCurrentDirection(bool bIsLeft);

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
};

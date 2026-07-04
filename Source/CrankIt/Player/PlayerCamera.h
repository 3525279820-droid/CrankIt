#pragma once

// 玩家四向视角 Pawn：输入转发 InputModeService，交互/电池由 Component 承担，字幕 HUD 交给 UIService

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "PlayerCamera.generated.h"

class UPlayerInteractionComponent;
class UBatteryHoldComponent;
class UKeyPromptWidgetBase;
class USkipTutorialWidget;
class USubtitleWidget;
class ULevelSequence;

// 玩家朝向索引变化时广播；IntroFlow 等可订阅，避免 GameMode Tick 轮询
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerDirectionChanged, int32);

UCLASS()
class CRANKIT_API APlayerCamera : public APawn
{
	GENERATED_BODY()

public:
	APlayerCamera();

	virtual void Tick(float DeltaTime) override;

	void InteractInput(const FInputActionValue& InputActionValue);
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void TurnInput(const FInputActionValue& value);
	void ExitScreenInput(const FInputActionValue& value);

	// 以下静态方法转发 UCrankItInputModeService，保留旧调用点
	static void ApplyExplorationInputMode(APlayerController* PC);
	static void SetExplorationMappingContextEnabled(APlayerController* PC, bool bEnabled);
	static void DisableAllInput(APlayerController* PC);
	static void EnableAllInput(APlayerController* PC);

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArmComp;

	// 玩家视角；ASoundDetectorActor 以其世界位置与朝前作为声音锥检测原点/正向
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* CameraComp;
	UPROPERTY(VisibleAnywhere)
	USceneComponent* SoundDetectorHoldPoint;

	// 相对基准位置向上抬升的最大距离（厘米）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SoundDetector|Hold")
	float SoundDetectorHoldLiftDistance = 20.f;

	// 抬升速度（厘米/秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SoundDetector|Hold")
	float SoundDetectorHoldLiftSpeed = 80.f;

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

	// 字幕 Widget 类；BeginPlay 时交给 UCrankItUIService 创建
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Subtitle")
	TSubclassOf<USubtitleWidget> SubtitleWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Tutorial")
	TSubclassOf<USkipTutorialWidget> SktWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Tutorial")
	TSubclassOf<UKeyPromptWidgetBase> KeyPromptWidgetClass;

	UPROPERTY()
	UKeyPromptWidgetBase* KeyPromptWidget = nullptr;

	UPROPERTY(EditAnywhere, Category = "Cinematic")
	TObjectPtr<ULevelSequence> CinematicSequence;

	UPROPERTY()
	AActor* OriginalViewTarget;

	FRotator DeltaRotation = FRotator::ZeroRotator;

	/** 过场/教程期间为 true；C++ 外部请用 SetInCinematic / IsInCinematic，勿直接写字段 */
	void SetInCinematic(bool bInCinematic) { bIsInCinematic = bInCinematic; }

	UFUNCTION(BlueprintPure, Category = "PlayerCamera|Cinematic")
	bool IsInCinematic() const { return bIsInCinematic; }

	int32 CurrentDirectionIndex = 2;

	UPROPERTY(EditAnywhere, Category = "PlayerCamera")
	TArray<FString> Directions = {"North", "East", "South", "West"};

	void UpdateCurrentDirection(bool bIsLeft);

	// 当前朝向索引（Directions 中 0=North … 3=West）；LightTrigger / IntroFlow 等读取
	UFUNCTION(BlueprintPure, Category = "PlayerCamera")
	int32 GetCurrentDirectionIndex() const { return CurrentDirectionIndex; }
	FOnPlayerDirectionChanged OnDirectionChanged;

protected:
	virtual void BeginPlay() override;

	bool bIsInCinematic = false;

	FVector SoundDetectorHoldBaseRelativeLocation = FVector::ZeroVector;
	float SoundDetectorHoldCurrentLift = 0.f;
	bool bLiftSoundDetectorHoldPoint = false;

	void UpdateSoundDetectorHoldLift(float DeltaTime);
};

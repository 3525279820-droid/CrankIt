#include "PlayerCamera.h"

#include "Input/CrankItInputModeService.h"
#include "UI/CrankItUIService.h"
#include "Player/PlayerInteractionComponent.h"
#include "Player/BatteryHoldComponent.h"
#include "GameFramework/PlayerController.h"

// 挂载 SpringArm、交互/电池 Component
APlayerCamera::APlayerCamera()
{
	PrimaryActorTick.bCanEverTick = true;
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SetRootComponent(SpringArmComp);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	BatteryHoldPoint = CreateDefaultSubobject<USceneComponent>(TEXT("BatteryHoldPoint"));
	BatteryHoldPoint->SetupAttachment(CameraComp);

	SoundDetectorHoldPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SoundDetectorHoldPoint"));
	SoundDetectorHoldPoint->SetupAttachment(CameraComp);

	InteractionComponent = CreateDefaultSubobject<UPlayerInteractionComponent>(TEXT("InteractionComponent"));
	BatteryHoldComponent = CreateDefaultSubobject<UBatteryHoldComponent>(TEXT("BatteryHoldComponent"));
}

// 转发至 UCrankItInputModeService（探索阶段默认输入）
void APlayerCamera::ApplyExplorationInputMode(APlayerController* PC)
{
	if (UCrankItInputModeService* Service = UCrankItInputModeService::GetFromController(PC))
	{
		Service->ApplyExplorationInputMode(PC);
	}
}

// 转发至 UCrankItInputModeService（教程/过场期间关输入）
void APlayerCamera::DisableAllInput(APlayerController* PC)
{
	if (UCrankItInputModeService* Service = UCrankItInputModeService::GetFromController(PC))
	{
		Service->DisableAllInput(PC);
	}
}

// 转发至 UCrankItInputModeService（教程/过场结束后恢复输入）
void APlayerCamera::EnableAllInput(APlayerController* PC)
{
	if (UCrankItInputModeService* Service = UCrankItInputModeService::GetFromController(PC))
	{
		Service->EnableAllInput(PC);
	}
}

// 转发至 UCrankItInputModeService（Enhanced Input 映射开关）
void APlayerCamera::SetExplorationMappingContextEnabled(APlayerController* PC, bool bEnabled)
{
	if (UCrankItInputModeService* Service = UCrankItInputModeService::GetFromController(PC))
	{
		Service->SetExplorationMappingContextEnabled(PC, bEnabled);
	}
}

// 初始化挂点、输入映射、经 UIService 创建字幕 HUD
void APlayerCamera::BeginPlay()
{
	Super::BeginPlay();

	if (SoundDetectorHoldPoint)
	{
		SoundDetectorHoldBaseRelativeLocation = SoundDetectorHoldPoint->GetRelativeLocation();
		SoundDetectorHoldCurrentLift = 0.f;
	}

	if (BatteryHoldComponent && BatteryHoldPoint)
	{
		BatteryHoldComponent->Initialize(BatteryHoldPoint);
	}

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		ApplyExplorationInputMode(PC);

		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
					ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
			{
				if (DefaultMappingContext)
				{
					Subsystem->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}

		// 字幕显示仍订阅 USubtitleSubsystem；Widget 生命周期由 UIService 统一管理
		if (UCrankItUIService* UIService = GetWorld()->GetSubsystem<UCrankItUIService>())
		{
			UIService->EnsureSubtitleWidget(PC, SubtitleWidgetClass, 200);
		}
	}
}

// 仅处理 SoundDetectorHoldPoint 抬升（悬停/电池逻辑在 InteractionComponent）
void APlayerCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bLiftSoundDetectorHoldPoint)
	{
		UpdateSoundDetectorHoldLift(DeltaTime);
	}
}

// 绑定 Enhanced Input：转向、交互拾取、退出电脑视角
void APlayerCamera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(TurnAction, ETriggerEvent::Started, this, &APlayerCamera::TurnInput);
		EIC->BindAction(IntereactAction, ETriggerEvent::Started, this, &APlayerCamera::InteractInput);
		EIC->BindAction(ExitScreen, ETriggerEvent::Started, this, &APlayerCamera::ExitScreenInput);
	}
}

// 交互键：从 InteractionComponent 取悬停电池，交给 BatteryHoldComponent 拾取
void APlayerCamera::InteractInput(const FInputActionValue& InputActionValue)
{
	if (BatteryHoldComponent && InteractionComponent)
	{
		BatteryHoldComponent->TryPickup(InteractionComponent->GetTargetBattery());
	}
}

// A/D 转向并更新 CurrentDirectionIndex，广播 OnDirectionChanged
void APlayerCamera::TurnInput(const FInputActionValue& value)
{
	if (bIsInCinematic)
	{
		return;
	}
	const float InputValue = value.Get<float>();
	if (InputValue > 0)
	{
		UpdateCurrentDirection(true);
	}
	else
	{
		UpdateCurrentDirection(false);
	}

	FRotator TargetRotation = FRotator::ZeroRotator;
	DeltaRotation.Yaw = InputValue * 90.f;
	TargetRotation.Yaw = SpringArmComp->GetComponentRotation().Yaw + DeltaRotation.Yaw;
	SpringArmComp->SetWorldRotation(TargetRotation);
}

// 从电脑固定相机切回 OriginalViewTarget（由 ComputerScreenActor 写入）
void APlayerCamera::ExitScreenInput(const FInputActionValue& value)
{
	if (!OriginalViewTarget)
	{
		return;
	}
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->SetViewTargetWithBlend(OriginalViewTarget, .5f, VTBlend_Cubic);
	}
	OriginalViewTarget = nullptr;
}

// 更新 Directions 环形索引并广播（IntroFlow 订阅 West=3 触发 Skip 教程）
void APlayerCamera::UpdateCurrentDirection(bool bIsLeft)
{
	if (bIsLeft)
	{
		if (CurrentDirectionIndex + 1 >= Directions.Num())
		{
			CurrentDirectionIndex = 0;
		}
		else
		{
			CurrentDirectionIndex++;
		}
	}
	else
	{
		if (CurrentDirectionIndex - 1 < 0)
		{
			CurrentDirectionIndex = Directions.Num() - 1;
		}
		else
		{
			CurrentDirectionIndex--;
		}
	}

	OnDirectionChanged.Broadcast(CurrentDirectionIndex);
}

// EMP 教程字幕结束后调用，Tick 中平滑抬升 SoundDetectorHoldPoint
void APlayerCamera::StartSoundDetectorHoldLift()
{
	if (SoundDetectorHoldCurrentLift >= SoundDetectorHoldLiftDistance - KINDA_SMALL_NUMBER)
	{
		return;
	}
	bLiftSoundDetectorHoldPoint = true;
}

// 按 SoundDetectorHoldLiftSpeed 插值抬升挂点相对 Z
void APlayerCamera::UpdateSoundDetectorHoldLift(float DeltaTime)
{
	if (!SoundDetectorHoldPoint || DeltaTime <= 0.f)
	{
		bLiftSoundDetectorHoldPoint = false;
		return;
	}

	const float Step = SoundDetectorHoldLiftSpeed * DeltaTime;
	SoundDetectorHoldCurrentLift += Step;
	SoundDetectorHoldCurrentLift = FMath::Clamp(SoundDetectorHoldCurrentLift, 0.f, SoundDetectorHoldLiftDistance);
	const FVector NewRelativeLocation =
		SoundDetectorHoldBaseRelativeLocation + FVector(0.f, 0.f, SoundDetectorHoldCurrentLift);
	SoundDetectorHoldPoint->SetRelativeLocation(NewRelativeLocation);

	if (SoundDetectorHoldCurrentLift >= SoundDetectorHoldLiftDistance - KINDA_SMALL_NUMBER)
	{
		bLiftSoundDetectorHoldPoint = false;
	}
}

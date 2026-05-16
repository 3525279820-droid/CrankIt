// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCamera.h"

#include <rapidjson/document.h>

// #include "ToolBuilderUtil.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SubtitleSubsystem.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "HAL/FileManager.h"
#include "Framework/Application/SlateApplication.h"

// Sets default values
APlayerCamera::APlayerCamera()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SetRootComponent(SpringArmComp);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	BatteryHoldPoint = CreateDefaultSubobject<USceneComponent>(TEXT("BatteryHoldPoint"));
	BatteryHoldPoint->SetupAttachment(CameraComp);

	SoundDetectorHoldPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SoundDetectorHoldPoint"));
	SoundDetectorHoldPoint->SetupAttachment(CameraComp);
	
}

void APlayerCamera::ApplyExplorationInputMode(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}
	// GameOnly + 可见鼠标时，左键常在 Slate 视口与「世界点击 / EI」之间被反复吞掉（全程如此，非仅开局）。
	// GameAndUI 且不指定 WidgetToFocus：仍把输入交给游戏，同时让鼠标按下能稳定参与 Hit/Click。
	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(Mode);
	PC->bShowMouseCursor = true;
	PC->bEnableClickEvents = true;
	PC->bEnableMouseOverEvents = true;
	FSlateApplication::Get().SetAllUserFocusToGameViewport(EFocusCause::SetDirectly);
}

void APlayerCamera::SetExplorationMappingContextEnabled(APlayerController* PC, bool bEnabled)
{
	if (!PC)
	{
		return;
	}
	APlayerCamera* Cam = Cast<APlayerCamera>(PC->GetPawn());
	if (!Cam || !Cam->DefaultMappingContext)
	{
		return;
	}
	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
	{
		if (bEnabled)
		{
			Subsystem->AddMappingContext(Cam->DefaultMappingContext, 0);
		}
		else
		{
			Subsystem->RemoveMappingContext(Cam->DefaultMappingContext);
		}
	}
}

// Called when the game starts or when spawned
void APlayerCamera::BeginPlay()
{
	Super::BeginPlay();

	if (SoundDetectorHoldPoint)
	{
		SoundDetectorHoldBaseRelativeLocation = SoundDetectorHoldPoint->GetRelativeLocation();
		SoundDetectorHoldCurrentLift = 0.f;
	}

	PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController)
	{
		ApplyExplorationInputMode(PlayerController);

		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
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
	}
	PlayerControllerRef = Cast<APlayerController>(APawn::GetController());

	TArray<AActor*> Found;
	MineConsole = Cast<AMineConsole>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AMineConsole::StaticClass()));

		
	if (PlayerController)
	{
		if (SubtitleWidgetClass)
		{
			SubtitlesWidget = CreateWidget<USubtitleWidget>(PlayerController, SubtitleWidgetClass);
		}
		else
		{
			SubtitlesWidget = CreateWidget<USubtitleWidget>(PlayerController);
		}
		if (SubtitlesWidget)
		{
			SubtitlesWidget->AddToPlayerScreen(200);
		}
	}
	
}

// Called every frame
void APlayerCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateBatteryPickupMotion(DeltaTime);

	FHitResult HitResult;

	if (PlayerControllerRef)
	{
		PlayerControllerRef->GetHitResultUnderCursor(
			ECollisionChannel::ECC_Visibility,
			false,
			HitResult
		);
		DrawDebugSphere(
			GetWorld(),
			HitResult.ImpactPoint,
			25.f,
			12,
			FColor::Red,
			false,
			-1.f
		);

		if (HoldBattery)
		{
			HoldBattery->RootComp->SetWorldLocation(BatteryHoldPoint->GetComponentLocation());
			HoldBattery->RootComp->SetWorldRotation(BatteryHoldPoint->GetComponentRotation());
		}
	}

	//检查鼠标指向对象
	
	UPrimitiveComponent* HitComp = HitResult.GetComponent();
	if(bIsInCinematic)
	{
		MineConsole->ShouldRotate = false;
	}
	
	if(not bIsInCinematic and HitComp)
	{

		if(HitComp->ComponentHasTag("ChargeHandle"))
		{
			if(MineConsole)
			{
				MineConsole->ShouldRotate = true;
			}	
		}

		else if(HitComp->ComponentHasTag("Battery"))
		{
			if(not HoldBattery and not BatteryMovingToHold)
			{
				TargetBattery = Cast<ABattery>(HitComp->GetOwner());
			}
		}
		else
		{
			if(MineConsole)
			{
				MineConsole->ShouldRotate = false;
				TargetBattery = nullptr;
			}
		}
	}
}



// Called to bind functionality to input
void APlayerCamera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if(EIC) {
		EIC->BindAction(TurnAction, ETriggerEvent::Started, this, &APlayerCamera::TurnInput);
		EIC->BindAction(IntereactAction, ETriggerEvent::Started, this, &APlayerCamera::InteractInput);
		EIC->BindAction(ExitScreen, ETriggerEvent::Started, this, &APlayerCamera::ExitScreenInput);
	}
}


void APlayerCamera::InteractInput(const FInputActionValue& InputActionValue)
{
	if (TargetBattery and not HoldBattery and not BatteryMovingToHold)
	{
		BatteryMovingToHold = TargetBattery;
		if (BatteryMovingToHold->RootComp)
		{
			BatteryPickupStartLoc = BatteryMovingToHold->RootComp->GetComponentLocation();
			BatteryPickupStartQuat = BatteryMovingToHold->RootComp->GetComponentQuat();
			BatteryPickupMoveAlpha = 0.f;
		}
		TargetBattery = nullptr;
	}
}

void APlayerCamera::UpdateBatteryPickupMotion(float DeltaTime)
{
	if (!BatteryMovingToHold || !BatteryHoldPoint)
	{
		return;
	}
	if (!IsValid(BatteryMovingToHold) || !BatteryMovingToHold->RootComp)
	{
		BatteryMovingToHold = nullptr;
		return;
	}

	if (DeltaTime <= 0.f)
	{
		return;
	}

	USceneComponent* Root = BatteryMovingToHold->RootComp;
	const FVector TargetLoc = BatteryHoldPoint->GetComponentLocation();
	const FQuat TargetQuat = BatteryHoldPoint->GetComponentQuat();

	const float Rate = FMath::Max(BatteryPickupLerpSpeed, KINDA_SMALL_NUMBER);
	BatteryPickupMoveAlpha = FMath::Clamp(BatteryPickupMoveAlpha + DeltaTime * Rate, 0.f, 1.f);

	const FVector NewLoc = FMath::Lerp(BatteryPickupStartLoc, TargetLoc, BatteryPickupMoveAlpha);
	const FQuat NewQuat = FQuat::Slerp(BatteryPickupStartQuat, TargetQuat, BatteryPickupMoveAlpha);
	Root->SetWorldLocationAndRotation(NewLoc, NewQuat);

	if (BatteryPickupMoveAlpha >= 1.f - KINDA_SMALL_NUMBER)
	{
		Root->SetWorldLocationAndRotation(TargetLoc, TargetQuat);
		HoldBattery = BatteryMovingToHold;
		BatteryMovingToHold = nullptr;
		BatteryPickupMoveAlpha = 0.f;
	}
}

void APlayerCamera::TurnInput(const FInputActionValue& value)
{
	float InputValue = value.Get<float>();
	FRotator TargetRotation = FRotator::ZeroRotator;

	DeltaRotation.Yaw = InputValue * 90.f;
	TargetRotation.Yaw = SpringArmComp->GetComponentRotation().Yaw + DeltaRotation.Yaw;
	
	SpringArmComp->SetWorldRotation(TargetRotation);
}

void APlayerCamera::ExitScreenInput(const FInputActionValue& value)
{
	if (OriginalViewTarget)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();

		PC->SetViewTargetWithBlend(OriginalViewTarget, .5f, VTBlend_Cubic);
		OriginalViewTarget = nullptr; // 清空，避免重复
	}
}


void APlayerCamera::PickBattery()
{
}

void APlayerCamera::MoveSoundDetectorHoldPointUp(float DeltaTime)
{
	if (!SoundDetectorHoldPoint || DeltaTime <= 0.f)
	{
		return;
	}

	const float Step = SoundDetectorHoldLiftSpeed * DeltaTime;
	SoundDetectorHoldCurrentLift += Step;
	SoundDetectorHoldCurrentLift = FMath::Clamp(SoundDetectorHoldCurrentLift, 0.f, SoundDetectorHoldLiftDistance);
	const FVector NewRelativeLocation =
		SoundDetectorHoldBaseRelativeLocation + FVector(0.f, 0.f, SoundDetectorHoldCurrentLift);
	SoundDetectorHoldPoint->SetRelativeLocation(NewRelativeLocation);
}

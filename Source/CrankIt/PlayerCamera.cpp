// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCamera.h"

#include <rapidjson/document.h>

// #include "ToolBuilderUtil.h"
#include "Kismet/GameplayStatics.h"

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

}

// Called when the game starts or when spawned
void APlayerCamera::BeginPlay()
{
	Super::BeginPlay();
	PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController) {
		ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
		if (LocalPlayer) {
			UEnhancedInputLocalPlayerSubsystem* Subsystem;
			Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
			if (Subsystem) {
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
	PlayerController->bEnableClickEvents = true;
	PlayerControllerRef = Cast<APlayerController>(APawn::GetController());

	TArray<AActor*> Found;
	MineConsole = Cast<AMineConsole>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AMineConsole::StaticClass()));

}

// Called every frame
void APlayerCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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

	if(HoldBattery)
	{
		HoldBattery->RootComp->SetWorldLocation(BatteryHoldPoint->GetComponentLocation());
		HoldBattery->RootComp->SetWorldRotation(BatteryHoldPoint->GetComponentRotation());
	}
	}

	//检查鼠标指向对象
	
	UPrimitiveComponent* HitComp = HitResult.GetComponent();
	
	if(HitComp)
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
			if(not HoldBattery)
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
	if(TargetBattery and not HoldBattery)
	{
		TargetBattery->RootComp->SetWorldLocation(BatteryHoldPoint->GetComponentLocation());
		TargetBattery->RootComp->SetWorldRotation(BatteryHoldPoint->GetComponentRotation());
		HoldBattery = TargetBattery;
		TargetBattery = nullptr;
		
		UE_LOG(LogTemp, Display, TEXT("Battery picked!"))
	}
}

void APlayerCamera::TurnInput(const FInputActionValue& value)
{
	UE_LOG(LogTemp, Display, TEXT("Should turn"))
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
	UE_LOG(LogTemp, Warning, TEXT("PlayerController: Tab action triggered"));
}


void APlayerCamera::PickBattery()
{
	UE_LOG(LogTemp, Display, TEXT("Battery clicked!"))
}


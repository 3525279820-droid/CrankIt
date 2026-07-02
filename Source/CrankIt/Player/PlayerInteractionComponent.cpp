#include "Player/PlayerInteractionComponent.h"

#include "Battery.h"
#include "Kismet/GameplayStatics.h"
#include "MineConsole.h"
#include "Player/BatteryHoldComponent.h"
#include "PlayerCamera.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

namespace
{
	const FName ChargeHandleTag(TEXT("ChargeHandle"));
	const FName BatteryTag(TEXT("Battery"));
}

UPlayerInteractionComponent::UPlayerInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// 缓存 MineConsole、PlayerController 与同 Pawn 上的 BatteryHoldComponent
void UPlayerInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World)
	{
		MineConsole = Cast<AMineConsole>(
			UGameplayStatics::GetActorOfClass(World, AMineConsole::StaticClass()));
	}

	if (AActor* Owner = GetOwner())
	{
		BatteryHoldComponent = Owner->FindComponentByClass<UBatteryHoldComponent>();

		if (APawn* OwnerPawn = Cast<APawn>(Owner))
		{
			PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
		}
		if (!PlayerController.IsValid() && World)
		{
			PlayerController = World->GetFirstPlayerController();
		}
	}
}

// 每帧鼠标射线检测，并更新悬停状态
void UPlayerInteractionComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PlayerController.IsValid())
	{
		if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
		{
			PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
		}
	}

	FHitResult HitResult;
	if (PlayerController.IsValid())
	{
		PlayerController->GetHitResultUnderCursor(
			ECollisionChannel::ECC_Visibility,
			false,
			HitResult);

		if (UWorld* World = GetWorld())
		{
			DrawDebugSphere(
				World,
				HitResult.ImpactPoint,
				25.f,
				12,
				FColor::Red,
				false,
				-1.f);
		}
	}

	UpdateHoverState(HitResult, IsInteractionEnabled());
}

// 同 Pawn 上的 UBatteryHoldComponent（BeginPlay 缓存）
UBatteryHoldComponent* UPlayerInteractionComponent::GetBatteryHold() const
{
	return BatteryHoldComponent.Get();
}

// 过场/教程期间（bIsInCinematic）关闭悬停交互
bool UPlayerInteractionComponent::IsInteractionEnabled() const
{
	if (const APlayerCamera* Cam = Cast<APlayerCamera>(GetOwner()))
	{
		return !Cam->bIsInCinematic;
	}
	return true;
}

// 状态变化时驱动 MineConsole 旋转并广播委托
void UPlayerInteractionComponent::SetChargeHandleHovered(bool bHovered)
{
	if (bChargeHandleHovered == bHovered)
	{
		return;
	}
	bChargeHandleHovered = bHovered;

	if (AMineConsole* Console = MineConsole.Get())
	{
		Console->SetShouldRotate(bHovered);
	}
	OnChargeHandleHoverChanged.Broadcast(bHovered);
}

// 根据鼠标命中组件更新 ChargeHandle 悬停与 Battery 拾取目标
void UPlayerInteractionComponent::UpdateHoverState(const FHitResult& HitResult, bool bInteractionEnabled)
{
	if (!bInteractionEnabled)
	{
		SetChargeHandleHovered(false);
		TargetBattery = nullptr;
		return;
	}

	UPrimitiveComponent* HitComp = HitResult.GetComponent();
	if (!HitComp)
	{
		SetChargeHandleHovered(false);
		TargetBattery = nullptr;
		return;
	}

	if (HitComp->ComponentHasTag(ChargeHandleTag))
	{
		SetChargeHandleHovered(true);
		TargetBattery = nullptr;
	}
	else if (HitComp->ComponentHasTag(BatteryTag))
	{
		UBatteryHoldComponent* HoldComp = GetBatteryHold();
		ABattery* Battery = Cast<ABattery>(HitComp->GetOwner());
		if (HoldComp && HoldComp->CanPickup(Battery))
		{
			TargetBattery = Battery;
		}
		else
		{
			TargetBattery = nullptr;
		}
	}
	else
	{
		SetChargeHandleHovered(false);
		TargetBattery = nullptr;
	}
}

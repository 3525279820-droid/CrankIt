#include "Player/BatteryHoldComponent.h"

#include "Battery.h"

UBatteryHoldComponent::UBatteryHoldComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// 绑定 Pawn 上的 BatteryHoldPoint 挂点（通常由 APlayerCamera::BeginPlay 调用）
void UBatteryHoldComponent::Initialize(USceneComponent* InHoldPoint)
{
	BatteryHoldPoint = InHoldPoint;
}

// 每帧推进拾取插值，并同步已持有电池到挂点
void UBatteryHoldComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdatePickupMotion(DeltaTime);
	UpdateHeldBatteryTransform();
}

// 手中无电池且未在插值移动时可拾取
bool UBatteryHoldComponent::CanPickup(ABattery* Battery) const
{
	return Battery && !HoldBattery.IsValid() && !BatteryMovingToHold.IsValid();
}

// 从悬停目标发起拾取，插值移向 BatteryHoldPoint
void UBatteryHoldComponent::TryPickup(ABattery* Battery)
{
	if (!CanPickup(Battery))
	{
		return;
	}

	BatteryMovingToHold = Battery;
	if (BatteryMovingToHold->RootComp)
	{
		PickupStartLoc = BatteryMovingToHold->RootComp->GetComponentLocation();
		PickupStartQuat = BatteryMovingToHold->RootComp->GetComponentQuat();
		PickupMoveAlpha = 0.f;
	}
}

// 槽位点击时取回手中电池并清空持有状态（槽位侧负责放回动画）
ABattery* UBatteryHoldComponent::ReleaseHeldBattery()
{
	ABattery* Released = HoldBattery.Get();
	HoldBattery = nullptr;
	BatteryMovingToHold = nullptr;
	PickupMoveAlpha = 0.f;
	return Released;
}

// 拾取插值：Lerp/Slerp 至挂点，完成后写入 HoldBattery
void UBatteryHoldComponent::UpdatePickupMotion(float DeltaTime)
{
	if (!BatteryMovingToHold.IsValid() || !BatteryHoldPoint.IsValid())
	{
		return;
	}
	if (!IsValid(BatteryMovingToHold.Get()) || !BatteryMovingToHold->RootComp)
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

	const float Rate = FMath::Max(PickupLerpSpeed, KINDA_SMALL_NUMBER);
	PickupMoveAlpha = FMath::Clamp(PickupMoveAlpha + DeltaTime * Rate, 0.f, 1.f);

	const FVector NewLoc = FMath::Lerp(PickupStartLoc, TargetLoc, PickupMoveAlpha);
	const FQuat NewQuat = FQuat::Slerp(PickupStartQuat, TargetQuat, PickupMoveAlpha);
	Root->SetWorldLocationAndRotation(NewLoc, NewQuat);

	if (PickupMoveAlpha >= 1.f - KINDA_SMALL_NUMBER)
	{
		Root->SetWorldLocationAndRotation(TargetLoc, TargetQuat);
		HoldBattery = BatteryMovingToHold;
		BatteryMovingToHold = nullptr;
		PickupMoveAlpha = 0.f;
	}
}

// 持有期间每帧对齐挂点
void UBatteryHoldComponent::UpdateHeldBatteryTransform()
{
	if (!HoldBattery.IsValid() || !BatteryHoldPoint.IsValid())
	{
		return;
	}
	if (!HoldBattery->RootComp)
	{
		return;
	}

	HoldBattery->RootComp->SetWorldLocation(BatteryHoldPoint->GetComponentLocation());
	HoldBattery->RootComp->SetWorldRotation(BatteryHoldPoint->GetComponentRotation());
}

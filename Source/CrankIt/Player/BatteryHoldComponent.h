#pragma once

// 电池拾取、挂点跟随与插值动画；BatterySlotTrigger 通过 ReleaseHeldBattery 取回。
// 由 APlayerCamera 挂载；BeginPlay 时 Initialize(BatteryHoldPoint) 绑定挂点。

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BatteryHoldComponent.generated.h"

class ABattery;
class USceneComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CRANKIT_API UBatteryHoldComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBatteryHoldComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Initialize(USceneComponent* InHoldPoint);

	ABattery* GetHeldBattery() const { return HoldBattery.Get(); }
	bool CanPickup(ABattery* Battery) const;

	void TryPickup(ABattery* Battery);
	ABattery* ReleaseHeldBattery();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battery|Pickup")
	float PickupLerpSpeed = 2.f;

private:
	void UpdatePickupMotion(float DeltaTime);
	void UpdateHeldBatteryTransform();

	TWeakObjectPtr<USceneComponent> BatteryHoldPoint;
	TWeakObjectPtr<ABattery> HoldBattery;
	TWeakObjectPtr<ABattery> BatteryMovingToHold;

	FVector PickupStartLoc = FVector::ZeroVector;
	FQuat PickupStartQuat = FQuat::Identity;
	float PickupMoveAlpha = 0.f;
};

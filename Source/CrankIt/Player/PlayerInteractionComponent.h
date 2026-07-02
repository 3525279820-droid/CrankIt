#pragma once

// 鼠标悬停检测：ChargeHandle 驱动 MineConsole 旋转，Battery 标签更新拾取目标。
// 由 APlayerCamera 挂载；InteractInput 读取 GetTargetBattery 后交给 UBatteryHoldComponent。

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerInteractionComponent.generated.h"

class AMineConsole;
class ABattery;
class APlayerController;
class UBatteryHoldComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnChargeHandleHoverChanged, bool);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CRANKIT_API UPlayerInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerInteractionComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	ABattery* GetTargetBattery() const { return TargetBattery.Get(); }
	bool IsChargeHandleHovered() const { return bChargeHandleHovered; }
	AMineConsole* GetMineConsole() const { return MineConsole.Get(); }

	FOnChargeHandleHoverChanged OnChargeHandleHoverChanged;

private:
	void UpdateHoverState(const FHitResult& HitResult, bool bInteractionEnabled);
	void SetChargeHandleHovered(bool bHovered);

	UBatteryHoldComponent* GetBatteryHold() const;
	bool IsInteractionEnabled() const;

	TWeakObjectPtr<AMineConsole> MineConsole;
	TWeakObjectPtr<ABattery> TargetBattery;
	TWeakObjectPtr<APlayerController> PlayerController;
	TWeakObjectPtr<UBatteryHoldComponent> BatteryHoldComponent;

	bool bChargeHandleHovered = false;
};

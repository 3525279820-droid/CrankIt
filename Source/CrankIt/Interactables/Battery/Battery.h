// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"

#include "Battery.generated.h"

UCLASS()
class CRANKIT_API ABattery : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABattery();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ShowChargeProgress();

	void ResetChargeProgress();

	/** 设置电量并同步指示灯（支持耗电时关灯） */
	void SetChargeProgress(int32 NewLevel);

	UFUNCTION(BlueprintPure, Category = "Battery")
	int32 GetChargeProgress() const { return ChargeProgress; }

	UFUNCTION(BlueprintPure, Category = "Battery")
	bool IsFullyCharged() const { return ChargeProgress >= 3; }

	/** 是否处于可充电槽位（ChargeSlot 重叠时为 true）；外部请用 SetChargingEnabled */
	UFUNCTION(BlueprintPure, Category = "Battery")
	bool IsChargingEnabled() const { return bCanCharge; }

	void SetChargingEnabled(bool bEnabled) { bCanCharge = bEnabled; }

	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootComp;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BatteryBase;
	
	UPROPERTY(VisibleAnywhere)
	TArray<UStaticMeshComponent*> BatteryChargeLevels;

	UPROPERTY(VisibleAnywhere)
	TArray<UPointLightComponent*> BatteryLights;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	int32 ChargeProgress = 0;

protected:
	/** 是否在 ChargeSlot 内可充电；Blueprint 只读，C++ 外部请用 SetChargingEnabled */
	UPROPERTY(BlueprintReadOnly, Category = "Battery")
	bool bCanCharge = false;

	virtual void BeginPlay() override;
};

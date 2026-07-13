// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Battery.generated.h"

class USoundBase;

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

	/** 设置电量并同步进度网格可见性（支持耗电时隐藏） */
	void SetChargeProgress(int32 NewLevel);

	UFUNCTION(BlueprintPure, Category = "Battery")
	int32 GetChargeProgress() const { return ChargeProgress; }

	UFUNCTION(BlueprintPure, Category = "Battery")
	bool IsFullyCharged() const { return ChargeProgress >= 3; }

	/** 是否处于可充电槽位（ChargeSlot 重叠时为 true）；外部请用 SetChargingEnabled */
	UFUNCTION(BlueprintPure, Category = "Battery")
	bool IsChargingEnabled() const { return bCanCharge; }

	void SetChargingEnabled(bool bEnabled) { bCanCharge = bEnabled; }

	void PlayInteractionSound();

	UPROPERTY(EditAnywhere, Category = "Battery|Audio")
	TObjectPtr<USoundBase> ChargeLevelSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "Battery|Audio")
	TObjectPtr<USoundBase> InteractionSound = nullptr;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootComp;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BatteryBase;
	
	UPROPERTY(VisibleAnywhere)
	TArray<UStaticMeshComponent*> BatteryChargeLevels;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	int32 ChargeProgress = 0;

protected:
	/** 是否在 ChargeSlot 内可充电；Blueprint 只读，C++ 外部请用 SetChargingEnabled */
	UPROPERTY(BlueprintReadOnly, Category = "Battery")
	bool bCanCharge = false;

	virtual void BeginPlay() override;
};

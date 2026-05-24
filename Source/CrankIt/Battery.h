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

	UPROPERTY(EditAnywhere)
	bool canCharge = false;


	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	


};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "Battery.h"
#include "BatterySlotTrigger.h"
#include "MineConsole.generated.h"

class UPointLightComponent;
UCLASS()
class CRANKIT_API AMineConsole : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMineConsole();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetShouldRotate(bool CanRotate);

	void StartLightingOffSequence();

	void LightNext();

	void LightOff();

	void AllLightsOff();

	void ChargeBattery();

	void CheckNeedCharge();
	
	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootComp;
	
	// UPROPERTY(VisibleAnywhere)
	// UStaticMeshComponent* ConsoleBase;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ChargeHandle;

	UPROPERTY(VisibleAnywhere)
	TArray<UPointLightComponent*> ChargeLights;

	UPROPERTY(VisibleAnywhere)
	TArray<UBatterySlotTrigger*> BatterySlots;


	
	FTimerHandle ChargeLightTimer;

	int32 CurrentLightIndex = 0;

	ABattery* Battery;

	TArray<AActor*> Batteries;

	

	bool ShouldRotate = false;
	
	float AngularVelocityYaw = 0.f;

	UPROPERTY(EditAnywhere)
	float SpinVelocity = 800.f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EMPLight.h"
#include "Battery.h"
#include "BatterySlotTrigger.h"
#include "Monster.h"

#include "LightTrigger.generated.h"

class USoundBase;

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))

class CRANKIT_API ULightTrigger : public UBoxComponent
{
	GENERATED_BODY()
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	ULightTrigger();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere)
	TArray<UBatterySlotTrigger*> BatterySlots;
	
	TArray<AActor*> AvailableBatteries;
	
	/** called when something leaves the sphere component */
	UFUNCTION()
	void OnButtonClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);
	AEMPLight* EMPLight;

	UPROPERTY(EditAnywhere, Category="Monster")
	AMonster* Monster;
	
	UPROPERTY(EditAnywhere, Category="Monster")
	FString CurrentDirection;

	UPROPERTY(EditAnywhere, Category="Battery")
	TArray<FName> SlotTags;   // 这个开关负责的槽位的标签集合（组件标签）

	UPROPERTY(EditAnywhere, Category = "LightTrigger|Audio")
	TObjectPtr<USoundBase> ButtonPressSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "LightTrigger|Audio")
	TObjectPtr<USoundBase> LightTriggerSound = nullptr;
};

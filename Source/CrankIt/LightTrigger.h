// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EMPLight.h"
#include "Battery.h"

#include "LightTrigger.generated.h"
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


	TArray<AActor*> AvailableBatteries;
	
	/** called when something leaves the sphere component */
	UFUNCTION()
	void OnButtonClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);
	AEMPLight* EMPLight;

	
};

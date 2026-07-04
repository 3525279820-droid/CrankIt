// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "EMPLight.h"
#include "LightTriggerComp.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CRANKIT_API ULightTriggerComp : public UBoxComponent
{
	GENERATED_BODY()
public:
	ULightTriggerComp();

	/** called when something leaves the sphere component */
	UFUNCTION()
	void OnButtonClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	
	AEMPLight* EMPLight;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};

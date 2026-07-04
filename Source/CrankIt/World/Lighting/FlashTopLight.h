// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/RectLightComponent.h"

#include "FlashTopLight.generated.h"

UCLASS()
class CRANKIT_API AFlashTopLight : public AActor
{
	GENERATED_BODY()
	
public:	
	AFlashTopLight();

	virtual void Tick(float DeltaTime) override;

	void LightStartFlash(float freq, int32 times);
	void SetLightOff();
	void SetLightOn();
	void SetIntensity(float Intensity);

	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootComp;

	UPROPERTY(VisibleAnywhere)
	URectLightComponent* TopLight;

	UPROPERTY(EditAnywhere, Category = "FlashTopLight")
	float FlashFreq = 0.2f;

	UPROPERTY(EditAnywhere, Category = "FlashTopLight")
	float FlashIntensity = 10000.f;

	UPROPERTY(EditAnywhere, Category = "FlashTopLight")
	int32 FlashTimes = 0;

protected:
	virtual void BeginPlay() override;

	FTimerHandle FlashTimerHandle;
	int32 RemainingFlashCycles = 0;
	float CurrentFlashFreq = 0.2f;

	void SetLightOffAndWait();
	void SetLightOnAndWait();
	void OnFlashCycleComplete();
};

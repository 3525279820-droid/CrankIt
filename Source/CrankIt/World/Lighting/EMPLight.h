// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SpotLightComponent.h"

#include "EMPLight.generated.h"

UCLASS()
class CRANKIT_API AEMPLight : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEMPLight();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ResetLight(float DeltaTime);
	
	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootComp;
	
	UPROPERTY(VisibleAnywhere)
	USpotLightComponent* EMPLightLeft;

	UPROPERTY(VisibleAnywhere)
	USpotLightComponent* EMPLightRight;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ButtonMesh;

	UPROPERTY(EditAnywhere)
	float LightIntensity = 0.f;

	FTimerHandle ResetLightHandle;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



};

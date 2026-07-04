// Fill out your copyright notice in the Description page of Project Settings.


#include "EMPLight.h"



// Sets default values
AEMPLight::AEMPLight()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root Comp"));
	SetRootComponent(RootComp);
	
	EMPLightLeft = CreateDefaultSubobject<USpotLightComponent>(TEXT("EMPLightLeft"));
	EMPLightLeft->SetupAttachment(RootComp);

	EMPLightRight = CreateDefaultSubobject<USpotLightComponent>("EMPLightRight");
	EMPLightRight->SetupAttachment(RootComp);

	ButtonMesh = CreateDefaultSubobject<UStaticMeshComponent>("ButtonMesh");
	ButtonMesh->SetupAttachment(RootComp);

	EMPLightRight->SetIntensity(LightIntensity);
	EMPLightLeft->SetIntensity(LightIntensity);

	
}

// Called when the game starts or when spawned
void AEMPLight::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AEMPLight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ResetLight(DeltaTime);

}

void AEMPLight::ResetLight(float DeltaTime)
{
	if(FMath::Abs(LightIntensity) > 0)
	{
		LightIntensity = FMath::FInterpTo(LightIntensity, 0.f, DeltaTime, 5.f);
		EMPLightLeft->SetIntensity(LightIntensity);
		EMPLightRight->SetIntensity(LightIntensity);

	}
}


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

void AEMPLight::BeginPlay()
{
	Super::BeginPlay();

	if (ButtonMesh)
	{
		ButtonRestRelativeLocation = ButtonMesh->GetRelativeLocation();
	}
}

void AEMPLight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ResetLight(DeltaTime);
	UpdateButtonPress(DeltaTime);
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

void AEMPLight::PlayButtonPress()
{
	if (!ButtonMesh)
	{
		return;
	}

	ButtonPressPhase = EButtonPressPhase::Pressing;
	ButtonPressAlpha = 0.f;
}

void AEMPLight::UpdateButtonPress(float DeltaTime)
{
	if (ButtonPressPhase == EButtonPressPhase::Idle || !ButtonMesh)
	{
		return;
	}

	if (ButtonPressPhase == EButtonPressPhase::Pressing)
	{
		const float Duration = FMath::Max(ButtonPressDownDuration, KINDA_SMALL_NUMBER);
		ButtonPressAlpha = FMath::Min(ButtonPressAlpha + DeltaTime / Duration, 1.f);
		if (ButtonPressAlpha >= 1.f - KINDA_SMALL_NUMBER)
		{
			ButtonPressPhase = EButtonPressPhase::Releasing;
		}
	}
	else
	{
		const float Duration = FMath::Max(ButtonPressReleaseDuration, KINDA_SMALL_NUMBER);
		ButtonPressAlpha = FMath::Max(ButtonPressAlpha - DeltaTime / Duration, 0.f);
		if (ButtonPressAlpha <= KINDA_SMALL_NUMBER)
		{
			ButtonPressAlpha = 0.f;
			ButtonPressPhase = EButtonPressPhase::Idle;
		}
	}

	const float PressOffset = ButtonPressAlpha * ButtonPressDistance;
	ButtonMesh->SetRelativeLocation(ButtonRestRelativeLocation - FVector(0.f, 0.f, PressOffset));
}

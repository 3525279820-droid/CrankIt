// Fill out your copyright notice in the Description page of Project Settings.


#include "FlashTopLight.h"

// Sets default values
AFlashTopLight::AFlashTopLight()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	TopLight = CreateDefaultSubobject<URectLightComponent>(TEXT("TopLight"));
	TopLight->SetupAttachment(RootComp);

	TopLight->SetIntensity(FlashIntensity);

}

// Called when the game starts or when spawned
void AFlashTopLight::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(FlashTimerHandle, [this]()
	{
		LightStartFlash(FlashFreq, FlashTimes);
	}, 5.f, false);
}

// Called every frame
void AFlashTopLight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFlashTopLight::LightStartFlash(float Freq, int32 Times)
{
	if (!TopLight || Times <= 0)
	{
		return;
	}

	CurrentFlashFreq = Freq;
	RemainingFlashCycles = Times;
	SetLightOffAndWait();
}

void AFlashTopLight::SetLightOffAndWait()
{
	if (!TopLight)
	{
		return;
	}
	TopLight->SetIntensity(0.f);
	GetWorldTimerManager().SetTimer(FlashTimerHandle, this, &AFlashTopLight::SetLightOnAndWait, CurrentFlashFreq, false);
}

void AFlashTopLight::SetLightOnAndWait()
{
	if (!TopLight)
	{
		return;
	}
	TopLight->SetIntensity(FlashIntensity);
	GetWorldTimerManager().SetTimer(FlashTimerHandle, this, &AFlashTopLight::OnFlashCycleComplete, CurrentFlashFreq, false);
}

void AFlashTopLight::OnFlashCycleComplete()
{
	RemainingFlashCycles--;
	if (RemainingFlashCycles > 0)
	{
		SetLightOffAndWait();
	}
}

void AFlashTopLight::SetLightOff()
{
	TopLight->SetIntensity(0.f);
}

void AFlashTopLight::SetLightOn()
{
	TopLight->SetIntensity(FlashIntensity);
}

void AFlashTopLight::SetIntensity(float NewIntensity)
{
	FlashIntensity = NewIntensity;
	if (TopLight)
	{
		TopLight->SetIntensity(NewIntensity);
	}
}
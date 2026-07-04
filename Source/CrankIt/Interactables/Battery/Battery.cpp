// Fill out your copyright notice in the Description page of Project Settings.


#include "Battery.h"

#include "PlayerCamera.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABattery::ABattery()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root Comp"));
	SetRootComponent(RootComp);

	BatteryBase = CreateDefaultSubobject<UStaticMeshComponent>("BatteryBase");
	BatteryBase->SetupAttachment(RootComp);

	for(int32 i = 0; i < 3; i++)
	{
		FName LightName = *FString::Printf(TEXT("BatteryLights_%d"), i);
		UPointLightComponent* PointLight = CreateDefaultSubobject<UPointLightComponent>(LightName);
		PointLight->SetupAttachment(BatteryBase);

		PointLight->SetVisibility(false);
		PointLight->SetIntensity(5000.f);
		PointLight->SetLightFColor(FColor(255, 0, 0, 255));

		BatteryLights.Add(PointLight);
	}
	
	for(int32 i = 0; i < 3; i++)
	{
		FName LightName = *FString::Printf(TEXT("BatteryChargeLevels_%d"), i);
		UStaticMeshComponent* StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(LightName);
		StaticMesh->SetupAttachment(BatteryBase);

		StaticMesh->SetVisibility(false);

		BatteryChargeLevels.Add(StaticMesh);
	}
}


// Called when the game starts or when spawned
void ABattery::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ABattery::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ShowChargeProgress();
}

void ABattery::ShowChargeProgress()
{
	if(ChargeProgress)
	{
		if(!BatteryLights[ChargeProgress - 1]->IsVisible())
		{
			BatteryLights[ChargeProgress - 1]->SetVisibility(true);
			UE_LOG(LogTemp, Display, TEXT("Battery Light on...."))
	
		}
	}
}

void ABattery::ResetChargeProgress()
{
	SetChargeProgress(0);
}

void ABattery::SetChargeProgress(int32 NewLevel)
{
	ChargeProgress = FMath::Clamp(NewLevel, 0, 3);
	for (int32 i = 0; i < BatteryLights.Num(); ++i)
	{
		const bool bOn = ChargeProgress > 0 && i < ChargeProgress;
		BatteryLights[i]->SetVisibility(bOn);
	}
	for (int32 i = 0; i < BatteryChargeLevels.Num(); ++i)
	{
		const bool bOn = ChargeProgress > 0 && i < ChargeProgress;
		BatteryChargeLevels[i]->SetVisibility(bOn);
	}
}




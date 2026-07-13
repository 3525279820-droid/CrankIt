// Fill out your copyright notice in the Description page of Project Settings.


#include "Battery.h"

#include "CrankItAudioService.h"

// Sets default values
ABattery::ABattery()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root Comp"));
	SetRootComponent(RootComp);

	BatteryBase = CreateDefaultSubobject<UStaticMeshComponent>("BatteryBase");
	BatteryBase->SetupAttachment(RootComp);

	for (int32 i = 0; i < 3; i++)
	{
		FName MeshName = *FString::Printf(TEXT("BatteryChargeLevels_%d"), i);
		UStaticMeshComponent* StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(MeshName);
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
	if (ChargeProgress <= 0)
	{
		return;
	}

	const int32 Idx = ChargeProgress - 1;
	if (!BatteryChargeLevels.IsValidIndex(Idx) || BatteryChargeLevels[Idx]->IsVisible())
	{
		return;
	}

	BatteryChargeLevels[Idx]->SetVisibility(true);
	UE_LOG(LogTemp, Display, TEXT("Battery charge level visible: %d"), Idx);
}

void ABattery::PlayInteractionSound()
{
	if (UCrankItAudioService* Audio = UCrankItAudioService::Get(this))
	{
		Audio->Play2D(InteractionSound);
	}
}

void ABattery::ResetChargeProgress()
{
	SetChargeProgress(0);
}

void ABattery::SetChargeProgress(int32 NewLevel)
{
	const int32 PreviousLevel = ChargeProgress;
	ChargeProgress = FMath::Clamp(NewLevel, 0, 3);
	for (int32 i = 0; i < 3; ++i)
	{
		const bool bOn = ChargeProgress > 0 && i < ChargeProgress;
		if (BatteryChargeLevels.IsValidIndex(i))
		{
			BatteryChargeLevels[i]->SetVisibility(bOn);
		}
	}

	if (ChargeProgress > PreviousLevel && ChargeProgress > 0)
	{
		if (UCrankItAudioService* Audio = UCrankItAudioService::Get(this))
		{
			Audio->Play2D(ChargeLevelSound);
		}
	}
}


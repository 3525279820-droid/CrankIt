// Fill out your copyright notice in the Description page of Project Settings.


#include "LightTrigger.h"

#include "EMPLight.h"


void ULightTrigger::BeginPlay()
{
	Super::BeginPlay();
	EMPLight = Cast<AEMPLight>(GetOwner());
	OnClicked.AddDynamic(this, &ULightTrigger::OnButtonClicked);
}

ULightTrigger::ULightTrigger()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULightTrigger::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void ULightTrigger::OnButtonClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	UE_LOG(LogTemp, Display, TEXT("Button be pressed"))
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABattery::StaticClass(), AvailableBatteries);
	for(int32 i = 0; i < 3; i++){
		ABattery* Battery = Cast<ABattery>(AvailableBatteries[i]);
		if(Battery)
		{
			if(Battery->ChargeProgress == 3)
			{
				if(EMPLight) EMPLight->LightIntensity = 25000.f;
				Battery->ChargeProgress = 0;
				Battery->ResetChargeProgress();
				break;
			}
		}
	}
}






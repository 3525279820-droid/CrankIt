// Fill out your copyright notice in the Description page of Project Settings.


#include "LightTriggerComp.h"

void ULightTriggerComp::BeginPlay()
{
	Super::BeginPlay();
	OnClicked.AddDynamic(this, &ULightTriggerComp::OnButtonClicked);
}

ULightTriggerComp::ULightTriggerComp()
{
	PrimaryComponentTick.bCanEverTick = true;
}
void ULightTriggerComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void ULightTriggerComp::OnButtonClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
}







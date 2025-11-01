// Fill out your copyright notice in the Description page of Project Settings.


#include "BatterySlotTrigger.h"

void UBatterySlotTrigger::BeginPlay()
{
	Super::BeginPlay();
	OnClicked.AddDynamic(this, &UBatterySlotTrigger::OnButtonClicked);
	OnComponentBeginOverlap.AddDynamic(this, &UBatterySlotTrigger::OnBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UBatterySlotTrigger::OnOverlapEnd);
}

UBatterySlotTrigger::UBatterySlotTrigger()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBatterySlotTrigger::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UBatterySlotTrigger::OnButtonClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	UE_LOG(LogTemp, Display, TEXT("Button be pressed"))
}

void UBatterySlotTrigger::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void UBatterySlotTrigger::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

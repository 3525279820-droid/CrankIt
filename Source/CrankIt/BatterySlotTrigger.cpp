// Fill out your copyright notice in the Description page of Project Settings.


#include "BatterySlotTrigger.h"

#include "Battery.h"
#include "PlayerCamera.h"

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
	APlayerCamera* Player = Cast<APlayerCamera>(GetWorld()->GetFirstPlayerController()->GetPawn());
	if (!Player) return;

	ABattery* HeldBattery = Player->HoldBattery;
	if (HeldBattery)
	{
		// 把电池移动到当前槽位（就是这个盒体的位置和旋转）
		HeldBattery->RootComp->SetWorldLocation(GetComponentLocation());
		HeldBattery->RootComp->SetWorldRotation(GetComponentRotation());

		// 更新状态
		Player->HoldBattery = nullptr;
		HeldBattery->canCharge = true;
	}
}

void UBatterySlotTrigger::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ABattery* Battery = Cast<ABattery>(OtherActor))
	{
		Battery->canCharge = true;
	}
}

void UBatterySlotTrigger::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ABattery* Battery = Cast<ABattery>(OtherActor))
	{
		Battery->canCharge = false;
	}
}

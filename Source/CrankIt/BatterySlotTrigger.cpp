// Fill out your copyright notice in the Description page of Project Settings.


#include "BatterySlotTrigger.h"

#include "Battery.h"
#include "Player/BatteryHoldComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

namespace
{
	static const FName ChargeSlotTag(TEXT("ChargeSlot"));
}

void UBatterySlotTrigger::BeginPlay()
{
	Super::BeginPlay();
	OnClicked.AddDynamic(this, &UBatterySlotTrigger::OnButtonClicked);
	OnComponentBeginOverlap.AddDynamic(this, &UBatterySlotTrigger::OnBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UBatterySlotTrigger::OnOverlapEnd);

	SyncOverlappingBatteryChargeState();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(this, &UBatterySlotTrigger::SyncOverlappingBatteryChargeState));
	}
}

void UBatterySlotTrigger::SyncOverlappingBatteryChargeState()
{
	if (!ComponentHasTag(ChargeSlotTag))
	{
		return;
	}
	TArray<AActor*> Overlapping;
	GetOverlappingActors(Overlapping, ABattery::StaticClass());
	for (AActor* Actor : Overlapping)
	{
		if (ABattery* Battery = Cast<ABattery>(Actor))
		{
			Battery->canCharge = true;
		}
	}
}

UBatterySlotTrigger::UBatterySlotTrigger()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBatterySlotTrigger::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateBatteryReturnMotion(DeltaTime);
}

// 槽位点击：从 Pawn 的 BatteryHoldComponent 取回电池并开始放回插值
void UBatterySlotTrigger::OnButtonClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	UBatteryHoldComponent* HoldComp = Pawn ? Pawn->FindComponentByClass<UBatteryHoldComponent>() : nullptr;
	if (!HoldComp)
	{
		return;
	}

	if (ReturningBattery)
	{
		return;
	}

	ABattery* HeldBattery = HoldComp->ReleaseHeldBattery();
	if (HeldBattery && HeldBattery->RootComp)
	{
		ReturningBattery = HeldBattery;

		BatteryReturnStartLoc = HeldBattery->RootComp->GetComponentLocation();
		BatteryReturnStartQuat = HeldBattery->RootComp->GetComponentQuat();
		BatteryReturnAlpha = 0.f;
	}
}

void UBatterySlotTrigger::UpdateBatteryReturnMotion(float DeltaTime)
{
	if (!IsValid(ReturningBattery) || !ReturningBattery->RootComp)
	{
		ReturningBattery = nullptr;
		return;
	}

	if (DeltaTime <= 0.f)
	{
		return;
	}

	const FVector TargetLoc = GetComponentLocation();
	const FQuat TargetQuat = GetComponentQuat();
	const float Rate = FMath::Max(BatteryReturnLerpSpeed, KINDA_SMALL_NUMBER);

	BatteryReturnAlpha = FMath::Clamp(BatteryReturnAlpha + DeltaTime * Rate, 0.f, 1.f);

	const FVector NewLoc = FMath::Lerp(BatteryReturnStartLoc, TargetLoc, BatteryReturnAlpha);
	const FQuat NewQuat = FQuat::Slerp(BatteryReturnStartQuat, TargetQuat, BatteryReturnAlpha);
	ReturningBattery->RootComp->SetWorldLocationAndRotation(NewLoc, NewQuat);

	if (BatteryReturnAlpha >= 1.f - KINDA_SMALL_NUMBER)
	{
		ReturningBattery->RootComp->SetWorldLocationAndRotation(TargetLoc, TargetQuat);
		ReturningBattery->canCharge = ComponentHasTag(ChargeSlotTag);
		ReturningBattery = nullptr;
		BatteryReturnAlpha = 0.f;
	}
}

void UBatterySlotTrigger::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!ComponentHasTag(ChargeSlotTag))
	{
		return;
	}
	if (ABattery* Battery = Cast<ABattery>(OtherActor))
	{
		Battery->canCharge = true;
	}
}

void UBatterySlotTrigger::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!ComponentHasTag(ChargeSlotTag))
	{
		return;
	}
	if (ABattery* Battery = Cast<ABattery>(OtherActor))
	{
		Battery->canCharge = false;
	}
}

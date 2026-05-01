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
	// UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABattery::StaticClass(), AvailableBatteries);
	UBatterySlotTrigger* SlotComp = MineConsole->FindComponentByClass<UBatterySlotTrigger>();
	if (!SlotComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("BatterySlotOwner has no UBoxComponent"));
		return;
	}
	
	if (!MineConsole || SlotTags.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Slot owner or tags not set for %s"), *GetName());
		return;
	}

	// 收集宿主 Actor 上的所有盒体槽位组件
	TArray<UBatterySlotTrigger*> AllSlotBoxes;
	MineConsole->GetComponents(AllSlotBoxes);


	// 只保留带有本开关指定标签的槽位
	TArray<UBatterySlotTrigger*> MySlots;
	for (UBatterySlotTrigger* Box : AllSlotBoxes)
	{
		for (const FName& Tag : SlotTags)
		{
			if (Box->ComponentHasTag(Tag))
			{
				MySlots.Add(Box);
				break;
			}
		}
	}
	
	// 查询MySlots中的电池
	for (UBoxComponent* Slot : MySlots)
	{
		TArray<AActor*> Overlapping;
		Slot->GetOverlappingActors(Overlapping, ABattery::StaticClass());

		for (AActor* Actor : Overlapping)
		{
			ABattery* Battery = Cast<ABattery>(Actor);
			if (!Battery) continue;

			if (Battery->ChargeProgress == 3)
			{
				if (EMPLight) EMPLight->LightIntensity = 25000.f;
				Battery->ChargeProgress = 0;
				Battery->ResetChargeProgress();
				if(CurrentDirection == Monster->CurrentDirection)
				{
					Monster->Repel();
				}
				return; // 找到一个满足条件的电池后就返回
			}
		}
	}
}






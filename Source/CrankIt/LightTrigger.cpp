// Fill out your copyright notice in the Description page of Project Settings.


#include "LightTrigger.h"

#include "EMPLight.h"
#include "EngineUtils.h"
#include "CrankItActorRegistry.h"
#include "MineConsole.h"
#include "PlayerCamera.h"
#include "GameFramework/PlayerController.h"

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
	// 从世界场景中收集所有盒体槽位组件
	TArray<UBatterySlotTrigger*> AllSlotBoxes;
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			TArray<UBatterySlotTrigger*> SlotComponents;
			It->GetComponents(SlotComponents);
			AllSlotBoxes.Append(SlotComponents);
		}
	}


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
				if (UWorld* World = GetWorld())
				{
					AMineConsole* Console = nullptr;
					int32 PlayerDirectionIndex = INDEX_NONE;

					if (UCrankItActorRegistry* Registry = World->GetSubsystem<UCrankItActorRegistry>())
					{
						Console = Registry->GetMineConsole();
					}

					if (APlayerController* PC = World->GetFirstPlayerController())
					{
						if (APlayerCamera* Cam = Cast<APlayerCamera>(PC->GetPawn()))
						{
							PlayerDirectionIndex = Cam->CurrentDirectionIndex;
						}
					}

					if (Console && Console->ShouldShowEMPTutorial(PlayerDirectionIndex))
					{
						Console->TryShowLightTutorialSubtitle();
					}
				}
			
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






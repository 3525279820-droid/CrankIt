// Fill out your copyright notice in the Description page of Project Settings.


#include "LightTrigger.h"

#include "EMPLight.h"
#include "EngineUtils.h"
#include "CrankItActorRegistry.h"
#include "CrankItAudioService.h"
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
	if (UCrankItAudioService* Audio = UCrankItAudioService::Get(this))
	{
		Audio->Play2D(ButtonPressSound);
	}

	if (EMPLight)
	{
		EMPLight->PlayButtonPress();
	}

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

			if (Battery->IsFullyCharged())
			{
				APlayerCamera* Cam = nullptr;
				UCrankItActorRegistry* Registry = nullptr;

				// 满电放电前：West 朝向且满足教程条件时播放 EMP 字幕轨（经 Registry + ShouldShowEMPTutorial）
				if (UWorld* World = GetWorld())
				{
					Registry = World->GetSubsystem<UCrankItActorRegistry>();

					AMineConsole* Console = nullptr;
					int32 PlayerDirectionIndex = INDEX_NONE;

					if (Registry)
					{
						Console = Registry->GetMineConsole();
					}

					if (APlayerController* PC = World->GetFirstPlayerController())
					{
						Cam = Cast<APlayerCamera>(PC->GetPawn());
						if (Cam)
						{
							PlayerDirectionIndex = Cam->GetCurrentDirectionIndex();
						}
					}

					if (Console && Console->ShouldShowEMPTutorial(PlayerDirectionIndex))
					{
						Console->TryShowLightTutorialSubtitle();
					}
				}

				if (UCrankItAudioService* Audio = UCrankItAudioService::Get(this))
				{
					Audio->Play2D(LightTriggerSound);
				}

				if (EMPLight) EMPLight->LightIntensity = 25000.f;
				Battery->ResetChargeProgress();

				// 驱怪：优先 Registry，未绑定则打日志跳过
				AMonster* TargetMonster = Monster;
				if (!IsValid(TargetMonster) && Registry)
				{
					TargetMonster = Registry->GetMonster();
				}
				if (!IsValid(TargetMonster))
				{
					UE_LOG(LogTemp, Warning, TEXT("LightTrigger: Monster not found."));
				}
				else
				{
					// 玩家当前朝向名与怪物所在方位比较；读不到玩家时回退到本开关配置的 CurrentDirection
					FString PlayerDirectionName = CurrentDirection;
					if (Cam)
					{
						const int32 Idx = Cam->GetCurrentDirectionIndex();
						if (Cam->Directions.IsValidIndex(Idx))
						{
							PlayerDirectionName = Cam->Directions[Idx];
						}
					}

					if (PlayerDirectionName == TargetMonster->CurrentDirection)
					{
						TargetMonster->Repel();
					}
				}
				return; // 找到一个满足条件的电池后就返回
			}
		}
	}
}






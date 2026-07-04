#include "BatteryPowerChecker.h"

#include "Battery.h"
#include "BatterySlotTrigger.h"
#include "EngineUtils.h"

const FName FBatteryPowerChecker::TerminalSlotTag(TEXT("TerminalSlot"));

void FBatteryPowerChecker::GetBatteriesInTerminalSlots(UWorld* World, int32 MinChargeLevel, TArray<ABattery*>& OutBatteries)
{
	OutBatteries.Reset();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		TArray<UBatterySlotTrigger*> Slots;
		It->GetComponents(Slots);
		for (UBatterySlotTrigger* Slot : Slots)
		{
			if (!Slot || !Slot->ComponentHasTag(TerminalSlotTag))
			{
				continue;
			}

			TArray<AActor*> Overlapping;
			Slot->GetOverlappingActors(Overlapping, ABattery::StaticClass());
			for (AActor* Actor : Overlapping)
			{
				if (ABattery* Battery = Cast<ABattery>(Actor))
				{
					if (Battery->GetChargeProgress() >= MinChargeLevel)
					{
						OutBatteries.AddUnique(Battery);
					}
				}
			}
		}
	}
}

bool FBatteryPowerChecker::HasEnoughPower(UWorld* World, int32 RequiredCount, int32 MinChargeLevel)
{
	TArray<ABattery*> Batteries;
	GetBatteriesInTerminalSlots(World, MinChargeLevel, Batteries);
	return Batteries.Num() >= RequiredCount;
}

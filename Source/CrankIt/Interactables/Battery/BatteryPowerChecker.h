#pragma once

#include "CoreMinimal.h"

class UWorld;
class UBatterySlotTrigger;
class ABattery;

/** TerminalSlot 槽位电池查询（纯静态，无 UObject） */
class FBatteryPowerChecker
{
public:
	static const FName TerminalSlotTag;

	/** 电量 >= MinChargeLevel 的电池数量是否达到 RequiredCount */
	static bool HasEnoughPower(UWorld* World, int32 RequiredCount, int32 MinChargeLevel);

	static void GetBatteriesInTerminalSlots(UWorld* World, int32 MinChargeLevel, TArray<ABattery*>& OutBatteries);
};

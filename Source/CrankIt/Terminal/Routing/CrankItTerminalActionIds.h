#pragma once

// 终端命令副作用 ID；Data Asset 中 CommandActionEntries / SequenceActionIds 须与此命名一致

#include "CoreMinimal.h"

namespace CrankItTerminalAction
{
	inline const FName OpenDoor(TEXT("OpenDoor"));
	inline const FName ClearTerminal(TEXT("ClearTerminal"));
	inline const FName Reboot(TEXT("Reboot"));
	inline const FName ScanAndRepair(TEXT("ScanAndRepair"));
	inline const FName Boordle(TEXT("Boordle"));
	inline const FName EnterClassificationGame(TEXT("EnterClassificationGame"));
	inline const FName Calibrate(TEXT("Calibrate"));
	inline const FName UpdateSystem(TEXT("UpdateSystem"));
	inline const FName LiftQuarantine(TEXT("LiftQuarantine"));
	inline const FName CalibrateNorthEntryDoor(TEXT("CalibrateNorthEntryDoor"));
	inline const FName GodIsDead(TEXT("GodIsDead"));
	inline const FName LiftOperational(TEXT("LiftOperational"));
	inline const FName Ascend(TEXT("Ascend"));
}

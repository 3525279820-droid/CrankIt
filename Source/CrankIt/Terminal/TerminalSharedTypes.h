#pragma once

#include "CoreMinimal.h"
#include "TerminalSharedTypes.generated.h"

UENUM(BlueprintType)
enum class ETerminalInputMode : uint8
{
	Terminal UMETA(DisplayName = "Terminal"),
	ClassificationGame UMETA(DisplayName = "ClassificationGame"),
	CalibrationGame UMETA(DisplayName = "CalibrationGame")
};

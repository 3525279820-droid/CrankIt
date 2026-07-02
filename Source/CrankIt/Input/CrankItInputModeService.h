#pragma once

// 集中管理探索阶段输入模式、Enhanced Input 映射上下文与过场/教程期间的输入开关。
// 旧入口 APlayerCamera 静态方法仍保留，内部转发至本子系统。

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrankItInputModeService.generated.h"

class APlayerController;

UCLASS()
class CRANKIT_API UCrankItInputModeService : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UCrankItInputModeService* GetFromController(APlayerController* PC);

	void ApplyExplorationInputMode(APlayerController* PC);
	void SetExplorationMappingContextEnabled(APlayerController* PC, bool bEnabled);
	void DisableAllInput(APlayerController* PC);
	void EnableAllInput(APlayerController* PC);
};

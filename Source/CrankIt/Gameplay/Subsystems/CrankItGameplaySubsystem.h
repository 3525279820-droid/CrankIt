#pragma once

// 关卡玩法事件总线：打破 MineConsole 与 GameMode / IntroFlow 之间的直接依赖

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrankItGameplaySubsystem.generated.h"

// EMP 教程字幕轨全部播完后广播；IntroFlow 订阅后解锁关卡
DECLARE_MULTICAST_DELEGATE(FOnPostEMPTutorialFinished);

UCLASS()
class CRANKIT_API UCrankItGameplaySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	FOnPostEMPTutorialFinished OnPostEMPTutorialFinished;

	// 由 AMineConsole 在 EMP 教程字幕 OnComplete 回调末尾调用
	void NotifyPostEMPTutorialFinished();

	// Skip 教程选 Yes 等路径统一经此写入 MineConsole（内部经 ActorRegistry 查找）
	void SetTutorialSkipped(bool bSkipped);
};

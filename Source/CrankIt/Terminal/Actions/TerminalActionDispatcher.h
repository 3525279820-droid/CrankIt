#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TerminalActionDispatcher.generated.h"

class UTerminalWidget;
class UTerminalDisplayController;

/** Terminal/Actions — 按 ActionId 执行命令副作用（改关卡、启动小游戏、编排输出） */
UCLASS()
class CRANKIT_API UTerminalActionDispatcher : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UTerminalWidget* InHost, UTerminalDisplayController* InDisplay);

	// CommandKey 为玩家输入的命令字符串，供 REBOOT 等从 CommandTextMap 取同步文案
	void Execute(FName ActionId, const FString& CommandKey, const TMap<FString, TArray<FString>>* CommandTextMap);

private:
	void RegisterActions();

	void AppendCommandTextFromMap(const FString& CommandKey, const TMap<FString, TArray<FString>>* CommandTextMap) const;

	void PlayCalibrateNorthEntryDoorSequence() const;

	UPROPERTY()
	TObjectPtr<UTerminalWidget> Host;

	UPROPERTY()
	TObjectPtr<UTerminalDisplayController> Display;

	TMap<FName, TFunction<void(const FString&, const TMap<FString, TArray<FString>>*)>> ActionHandlers;
};

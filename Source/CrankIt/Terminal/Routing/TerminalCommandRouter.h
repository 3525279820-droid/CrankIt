#pragma once

// Terminal/Routing — 命令阶段校验与 ActionId / 文本表查表（纯 C++，无 UObject）

#include "CoreMinimal.h"

enum class ETerminalSubmitStatus : uint8
{
	WrongStage,       // 输入与 CommandSequence[NextIndex] 不一致
	PastEnd,          // 主线命令已全部完成
	AcceptedTextOnly, // 命中 CommandTextMap，无 ActionId
	AcceptedAction,   // 命中 ActionId，由 Dispatcher 执行
	AcceptedUnknown   // 阶段正确但未配置文本或行为（legacy 分支）
};

struct FTerminalCommandSubmitResult
{
	ETerminalSubmitStatus Status = ETerminalSubmitStatus::WrongStage;
	FName ActionId = NAME_None;
	TArray<FString> TextLines;
	FString MatchedCommand;
};

// Router 初始化快照（由 NarrativeSubsystem 从 TerminalCommandData 填充）
struct FTerminalCommandRouterConfig
{
	TArray<FString> CommandSequence;
	TMap<FString, TArray<FString>> CommandTextMap;
	TMap<FString, FName> CommandToActionId;
	// 与 CommandSequence 等长；非 None 时优先于 CommandToActionId
	TArray<FName> SequenceActionIds;
};

class FTerminalCommandRouter
{
public:
	void LoadConfig(const FTerminalCommandRouterConfig& Config);
	static void ApplyDefaultCommandToActionMap(TMap<FString, FName>& OutMap);

	FTerminalCommandSubmitResult Submit(const FString& Input);

	void ResetProgress();
	int32 GetNextCommandIndex() const { return NextCommandIndex; }
	const TMap<FString, TArray<FString>>& GetCommandTextMap() const { return CommandTextMap; }

private:
	FName ResolveActionIdForIndex(int32 Index, const FString& Command) const;

	TArray<FString> CommandSequence;
	TMap<FString, TArray<FString>> CommandTextMap;
	TMap<FString, FName> CommandToActionId;
	TArray<FName> SequenceActionIds;
	int32 NextCommandIndex = 0;
};

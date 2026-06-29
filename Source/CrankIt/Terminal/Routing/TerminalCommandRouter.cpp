#include "TerminalCommandRouter.h"

#include "CrankItTerminalActionIds.h"

// Data Asset 未配置 CommandActionEntries 时的 fallback，与旧 CommandActionMap 非空键一致
void FTerminalCommandRouter::ApplyDefaultCommandToActionMap(TMap<FString, FName>& OutMap)
{
	OutMap.Add(TEXT("REBOOT"), CrankItTerminalAction::Reboot);
	OutMap.Add(TEXT("SCAN AND REPAIR"), CrankItTerminalAction::ScanAndRepair);
	OutMap.Add(TEXT("BOORDLE"), CrankItTerminalAction::Boordle);
	OutMap.Add(TEXT("CALIBRATE"), CrankItTerminalAction::Calibrate);
	OutMap.Add(TEXT("UPDATE SYSTEM"), CrankItTerminalAction::UpdateSystem);
	OutMap.Add(TEXT("LIFT QUARANTINE"), CrankItTerminalAction::LiftQuarantine);
	OutMap.Add(TEXT("CALIBRATE NORTH ENTRY DOOR"), CrankItTerminalAction::CalibrateNorthEntryDoor);
	OutMap.Add(TEXT("ASCEND"), CrankItTerminalAction::Ascend);
}

// 载入 Data Asset 快照并重置阶段进度
void FTerminalCommandRouter::LoadConfig(const FTerminalCommandRouterConfig& Config)
{
	CommandSequence = Config.CommandSequence;
	CommandTextMap = Config.CommandTextMap;
	CommandToActionId = Config.CommandToActionId;
	SequenceActionIds = Config.SequenceActionIds;
	NextCommandIndex = 0;

	if (CommandToActionId.Num() == 0)
	{
		ApplyDefaultCommandToActionMap(CommandToActionId);
	}
}

// 解析当前阶段应对应的 ActionId：SequenceActionIds 优先，其次 CommandToActionId
FName FTerminalCommandRouter::ResolveActionIdForIndex(int32 Index, const FString& Command) const
{
	if (SequenceActionIds.IsValidIndex(Index) && !SequenceActionIds[Index].IsNone())
	{
		return SequenceActionIds[Index];
	}

	if (const FName* ActionId = CommandToActionId.Find(Command))
	{
		return *ActionId;
	}

	return NAME_None;
}

// 校验玩家输入是否匹配当前阶段期望命令，并返回 Action / 纯文本 / 错误状态
FTerminalCommandSubmitResult FTerminalCommandRouter::Submit(const FString& Input)
{
	FTerminalCommandSubmitResult Result;
	Result.MatchedCommand = Input;

	if (NextCommandIndex >= CommandSequence.Num())
	{
		Result.Status = ETerminalSubmitStatus::PastEnd;
		return Result;
	}

	const FString& Expected = CommandSequence[NextCommandIndex];
	if (!Input.Equals(Expected))
	{
		Result.Status = ETerminalSubmitStatus::WrongStage;
		return Result;
	}

	const FName ActionId = ResolveActionIdForIndex(NextCommandIndex, Input);
	if (!ActionId.IsNone())
	{
		Result.Status = ETerminalSubmitStatus::AcceptedAction;
		Result.ActionId = ActionId;
		++NextCommandIndex;
		return Result;
	}

	if (const TArray<FString>* Lines = CommandTextMap.Find(Input))
	{
		Result.Status = ETerminalSubmitStatus::AcceptedTextOnly;
		Result.TextLines = *Lines;
		++NextCommandIndex;
		return Result;
	}

	// 阶段正确但未配置 Action 与文本（与原 Unknown command 分支一致）
	Result.Status = ETerminalSubmitStatus::AcceptedUnknown;
	++NextCommandIndex;
	return Result;
}

// 重置主线进度（如需重开终端流程时调用）
void FTerminalCommandRouter::ResetProgress()
{
	NextCommandIndex = 0;
}

#include "CrankItTerminalCommandData.h"

// 返回 Primary Asset 标识，供资产管理系统识别
FPrimaryAssetId UCrankItTerminalCommandData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("CrankItTerminalCommand"), GetFName());
}

// 将 CommandTextEntries 转为运行时 TMap，供 CommitInput 查询
void UCrankItTerminalCommandData::BuildCommandTextMap(TMap<FString, TArray<FString>>& OutMap) const
{
	OutMap.Empty();
	for (const FCrankItTerminalCommandTextEntry& Entry : CommandTextEntries)
	{
		if (!Entry.Command.IsEmpty())
		{
			OutMap.Add(Entry.Command, Entry.Lines);
		}
	}
}

// 按 BlockId 查找终端输出块
bool UCrankItTerminalCommandData::GetOutputBlock(FName BlockId, TArray<FString>& OutLines) const
{
	if (BlockId.IsNone())
	{
		return false;
	}

	if (const FCrankItTerminalOutputBlock* Block = OutputBlocks.Find(BlockId))
	{
		OutLines = Block->Lines;
		return OutLines.Num() > 0;
	}
	return false;
}

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CrankItTerminalCommandData.generated.h"

// 终端命令 → 纯文本多行输出（CommitInput 命中 CommandTextMap 时使用）
USTRUCT(BlueprintType)
struct CRANKIT_API FCrankItTerminalCommandTextEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terminal")
	FString Command;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terminal")
	TArray<FString> Lines;
};

// 终端输出块：CommandAction / ComputerScreen 等按 BlockId 引用
USTRUCT(BlueprintType)
struct CRANKIT_API FCrankItTerminalOutputBlock
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terminal")
	TArray<FString> Lines;
};

// 终端命令与控制台输出 Primary Data Asset（编辑器中创建 DA_CrankItTerminalCommands）
UCLASS(BlueprintType)
class CRANKIT_API UCrankItTerminalCommandData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	// 主线命令输入顺序
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terminal")
	TArray<FString> CommandSequence;

	// 命令 → 纯文本响应（无玩法副作用）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terminal")
	TArray<FCrankItTerminalCommandTextEntry> CommandTextEntries;

	// 输出块；BlockId 见 CrankItNarrativeIds.h
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terminal")
	TMap<FName, FCrankItTerminalOutputBlock> OutputBlocks;

	// 构建运行时 CommandTextMap
	void BuildCommandTextMap(TMap<FString, TArray<FString>>& OutMap) const;
	// 按 BlockId 查找输出块
	bool GetOutputBlock(FName BlockId, TArray<FString>& OutLines) const;
};

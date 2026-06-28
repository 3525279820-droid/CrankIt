#include "CrankItNarrativeSubsystem.h"

#include "CrankItNarrativeData.h"
#include "CrankItTerminalCommandData.h"
#include "InitLevel.h"
#include "GameFramework/GameModeBase.h"

// 注入字幕剧本 Data Asset（通常由 AInitLevel::BeginPlay 调用）
void UCrankItNarrativeSubsystem::SetNarrativeData(UCrankItNarrativeData* InData)
{
	NarrativeData = InData;
}

// 注入终端命令 Data Asset（通常由 AInitLevel::BeginPlay 调用）
void UCrankItNarrativeSubsystem::SetTerminalCommandData(UCrankItTerminalCommandData* InData)
{
	TerminalCommandData = InData;
}

// 若尚未注入，则从 GameMode 懒加载 Data Asset 引用
void UCrankItNarrativeSubsystem::RefreshDataFromGameMode() const
{
	if (NarrativeData && TerminalCommandData)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (AInitLevel* InitLevel = World->GetAuthGameMode<AInitLevel>())
	{
		if (!NarrativeData)
		{
			NarrativeData = InitLevel->NarrativeData;
		}
		if (!TerminalCommandData)
		{
			TerminalCommandData = InitLevel->TerminalCommandData;
		}
	}
}

// 按 TrackId 播放字幕轨，结束后可选执行 OnComplete
void UCrankItNarrativeSubsystem::PlaySubtitleTrack(FName TrackId, TFunction<void()> OnComplete)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		if (OnComplete)
		{
			OnComplete();
		}
		return;
	}

	TArray<FCrankItSubtitleLine> Lines;
	if (!GetSubtitleTrackLines(TrackId, Lines))
	{
		UE_LOG(LogTemp, Warning, TEXT("CrankItNarrative: Subtitle track '%s' not found or empty. Fill DA_CrankItNarrative."), *TrackId.ToString());
		if (OnComplete)
		{
			OnComplete();
		}
		return;
	}

	// 实际播放入口仍走 USubtitleSubsystem
	if (USubtitleSubsystem* SubtitleSys = World->GetSubsystem<USubtitleSubsystem>())
	{
		SubtitleSys->PlaySubtitleTrack(Lines, MoveTemp(OnComplete));
	}
	else if (OnComplete)
	{
		OnComplete();
	}
}

// 查询字幕轨文本，供 PlaySubtitleTrack 或外部读取
bool UCrankItNarrativeSubsystem::GetSubtitleTrackLines(FName TrackId, TArray<FCrankItSubtitleLine>& OutLines) const
{
	OutLines.Empty();
	RefreshDataFromGameMode();

	if (NarrativeData)
	{
		return NarrativeData->GetSubtitleTrack(TrackId, OutLines);
	}
	return false;
}

// 查询终端输出块（BlockId 见 CrankItNarrativeIds.h）
bool UCrankItNarrativeSubsystem::GetTerminalOutputLines(FName BlockId, TArray<FString>& OutLines) const
{
	OutLines.Empty();
	RefreshDataFromGameMode();

	if (TerminalCommandData)
	{
		return TerminalCommandData->GetOutputBlock(BlockId, OutLines);
	}
	return false;
}

// 将 Data Asset 中的命令顺序与文本表写入终端 Widget
bool UCrankItNarrativeSubsystem::ApplyTerminalCommandData(
	TArray<FString>& OutSequence,
	TMap<FString, TArray<FString>>& OutTextMap) const
{
	RefreshDataFromGameMode();

	if (!TerminalCommandData)
	{
		UE_LOG(LogTemp, Warning, TEXT("CrankItNarrative: TerminalCommandData not assigned on GameMode."));
		return false;
	}

	OutSequence = TerminalCommandData->CommandSequence;
	TerminalCommandData->BuildCommandTextMap(OutTextMap);

	if (OutSequence.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("CrankItNarrative: CommandSequence is empty in TerminalCommandData."));
		return false;
	}
	return true;
}

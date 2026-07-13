#include "CrankItNarrativeData.h"

// 返回 Primary Asset 标识，供资产管理系统识别
FPrimaryAssetId UCrankItNarrativeData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("CrankItNarrative"), GetFName());
}

// 按 TrackId 查找字幕轨并写入 OutLines
bool UCrankItNarrativeData::GetSubtitleTrack(FName TrackId, TArray<FCrankItSubtitleLine>& OutLines) const
{
	if (TrackId.IsNone())
	{
		return false;
	}

	for (const FCrankItSubtitleTrackEntry& Entry : SubtitleTracks)
	{
		if (Entry.TrackId == TrackId)
		{
			OutLines = Entry.Lines;
			// 空轨视为未找到
			return OutLines.Num() > 0;
		}
	}
	return false;
}

USoundBase* UCrankItNarrativeData::GetSubtitleVoice(FName TrackId) const
{
	if (TrackId.IsNone())
	{
		return nullptr;
	}

	for (const FCrankItSubtitleTrackEntry& Entry : SubtitleTracks)
	{
		if (Entry.TrackId == TrackId)
		{
			return Entry.Voice;
		}
	}
	return nullptr;
}

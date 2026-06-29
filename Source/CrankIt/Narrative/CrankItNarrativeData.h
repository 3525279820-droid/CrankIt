#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SubtitleSubsystem.h"
#include "CrankItNarrativeData.generated.h"

// 单条字幕轨：TrackId + 时间轴文本列表
USTRUCT(BlueprintType)
struct CRANKIT_API FCrankItSubtitleTrackEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
	FName TrackId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
	TArray<FCrankItSubtitleLine> Lines;
};

// 字幕剧本 Primary Data Asset（编辑器中创建 DA_CrankItNarrative）
UCLASS(BlueprintType)
class CRANKIT_API UCrankItNarrativeData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	// 字幕轨列表；TrackId 见 CrankItNarrativeIds.h
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
	TArray<FCrankItSubtitleTrackEntry> SubtitleTracks;

	// 按 TrackId 查找字幕轨
	bool GetSubtitleTrack(FName TrackId, TArray<FCrankItSubtitleLine>& OutLines) const;
};

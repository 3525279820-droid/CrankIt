#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SubtitleSubsystem.h"
#include "CrankItNarrativeData.generated.h"

class USoundBase;

// 单条字幕轨：TrackId + 时间轴文本列表
USTRUCT(BlueprintType)
struct CRANKIT_API FCrankItSubtitleTrackEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
	FName TrackId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
	TArray<FCrankItSubtitleLine> Lines;

	// 该字幕轨对应的语音文件
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative|Voice")
	TObjectPtr<USoundBase> Voice = nullptr;
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

	// 按字幕 TrackId 查找语音资源；无配置时返回 nullptr
	USoundBase* GetSubtitleVoice(FName TrackId) const;
};

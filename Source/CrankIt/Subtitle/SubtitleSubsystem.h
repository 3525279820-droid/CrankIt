#pragma once

/**
 * 字幕系统（思路概要）
 *
 * 1. 时间源：以「当前播到哪一秒」为唯一时钟。UE 5.6 起 UAudioComponent 不再提供 GetPlaybackPercent，
 *    改为绑定 OnAudioPlaybackPercentNative 缓存 0~1 进度，再 × GetSound()->GetDuration() 得到秒；
 *    无音频时退化为「从 Start 调用时刻起」的世界时间秒；2D 语音若进度回调不触发，同样回退为世界时间。
 *
 * 2. 数据：每条 FCrankItSubtitleCue 表示半开区间 [StartTimeSeconds, EndTimeSeconds)，在此区间内显示 Text。
 *    （命名避开引擎自带的 FSubtitleCue。）
 *    轨开始时会按开始时间排序，便于线性扫描；若两条时间重叠，先出现的条目优先匹配。
 *
 * 3. 驱动与 UI 解耦：USubtitleSubsystem（World 子系统 + 每帧 Tick）只负责算当前句并通过多播委托广播；
 *    任意 UMG 可绑定，USubtitleWidget 只是默认的一种「订阅 + TextBlock」实现。
 */

#include "CoreMinimal.h"
#include <atomic>

// UE 5.5+：UTickableWorldSubsystem 声明于 WorldSubsystem.h（已无 TickableWorldSubsystem.h）。
#include "Subsystems/WorldSubsystem.h"
#include "CrankItAudioService.h"
#include "SubtitleSubsystem.generated.h"

class UAudioComponent;
class USoundBase;
class USoundWave;

/** 单条字幕输入（{Start, End, Text}），供 PlaySubtitleTrack 转为 FCrankItSubtitleCue。 */
USTRUCT(BlueprintType)
struct CRANKIT_API FCrankItSubtitleLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
	float StartTimeSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
	float EndTimeSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
	FString Text;
};

/** 单条字幕：时间与文本；区间为 [Start, End)（End 秒那一瞬起不再显示本条）。不可命名 FSubtitleCue，与引擎类型冲突。 */
USTRUCT(BlueprintType)
struct CRANKIT_API FCrankItSubtitleCue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
	float StartTimeSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
	float EndTimeSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
	FText Text;
};

/** 字幕行变化（含清空）：供蓝图绑定。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSubtitleLineChanged, FText, SubtitleText);

/**
 * 每帧根据「音频播放秒数」或「世界时间经过秒数」更新当前字幕，并去重后广播。
 * UI 侧只监听委托，不直接读 AudioComponent，避免界面与具体发声 Actor 强耦合。
 */
UCLASS()
class CRANKIT_API USubtitleSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	/** 当前应显示的字幕变化时触发；无匹配时为 FText::GetEmpty()（可用来隐藏）。 */
	UPROPERTY(BlueprintAssignable, Category = "Subtitle")
	FOnSubtitleLineChanged OnSubtitleLineChanged;

	/** 开始一组字幕，时间与 SyncAudio 的播放进度同步。 */
	UFUNCTION(BlueprintCallable, Category = "Subtitle")
	void StartSubtitleTrack(const TArray<FCrankItSubtitleCue>& Cues, UAudioComponent* SyncAudio);

	/** 无音频时：以调用时刻的世界时间为 0 秒推进（便于测试或外部时间轴）。 */
	UFUNCTION(BlueprintCallable, Category = "Subtitle")
	void StartSubtitleTrackWithWorldTime(const TArray<FCrankItSubtitleCue>& Cues);

	/**
	 * 播放字幕轨；有 Voice 时与语音进度对齐，无 Voice 时退化为世界时间轴。
	 * Lines 为空时立即调用 OnComplete。
	 */
	void PlaySubtitleTrack(
		const TArray<FCrankItSubtitleLine>& Lines,
		USoundBase* Voice = nullptr,
		TFunction<void()> OnComplete = TFunction<void()>());

	UFUNCTION(BlueprintCallable, Category = "Subtitle")
	void StopSubtitles();

	UFUNCTION(BlueprintPure, Category = "Subtitle")
	bool IsSubtitleTrackActive() const { return bTrackActive; }

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickableWhenPaused() const override { return false; }
	virtual bool IsTickableInEditor() const override { return false; }

protected:
	/** 音频线程/混音回调可能写入，GameThread Tick 读取；用 atomic 避免数据竞争。 */
	void OnAudioPlaybackPercentNative(const UAudioComponent* InAudioComponent, const USoundWave* PlayingSoundWave, float PlaybackPercent);

	/** 使用缓存的播放百分比 × 资源时长得到当前秒（与 Cue 表对齐）。 */
	float GetSyncedPlaybackSeconds(UAudioComponent* AudioComp) const;

	/** 与上一句比较，避免每帧重复触发 UI 刷新。 */
	void SetCurrentSubtitleText(const FText& NewText);

	/** 根据当前秒数选中一条 Cue，并在整条轨结束后关闭子系统状态。 */
	void UpdateForTime(float PlaybackSeconds);

	/** 遍历查找当前时间落入的区间，然后返回Cue的索引。 */
	int32 FindCueIndexForTime(float T) const;

	/** 轨自然结束或定时器触发：清理字幕并执行 PlaySubtitleTrack 传入的 OnComplete（仅一次）。 */
	void FinishSubtitleTrack();

	void StopSubtitles(UAudioComponent* PreserveVoiceComp);

	TArray<FCrankItSubtitleCue> ActiveCues;
	/** 弱引用：音频 Actor 销毁时不拖住对象，Tick 里需判有效性。 */
	TWeakObjectPtr<UAudioComponent> SyncAudioWeak;
	bool bUseWorldTime = false;
	/** 世界时间模式下的 t=0 时刻（GetTimeSeconds）。 */
	double TrackStartWorldTimeSeconds = 0.0;
	bool bTrackActive = false;
	/** 用于 EqualTo 去重广播。 */
	FText LastBroadcastText;

	/** 由 OnAudioPlaybackPercentNative 更新，供 Tick 换算为秒。 */
	mutable std::atomic<float> CachedPlaybackPercent{0.f};

	/** 由 CrankItAudioService::Play2DTracked 播放的字幕语音；StopSubtitles 时经 AudioService 停止 */
	FCrankItSoundHandle VoiceSoundHandle;

	FTimerHandle TrackCompleteTimer;

	/** PlaySubtitleTrack 注册的结束回调；StopSubtitles 会取消（不调用）。 */
	TFunction<void()> PendingTrackOnComplete;
};

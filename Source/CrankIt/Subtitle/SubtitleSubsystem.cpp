// 实现要点：Tick 中只读「当前播放秒」→ FindCueIndex → 有变化才 Broadcast；
// 结束条件二选一——音频已停，或播放时间已超过最后一条 End（留少量余量避免边界抖动）。
// UE 5.6：播放进度依赖 OnAudioPlaybackPercentNative 写入的 CachedPlaybackPercent。

#include "SubtitleSubsystem.h"
#include "SubtitleWidget.h"

#include "CrankItAudioService.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "Stats/Stats.h"

DEFINE_LOG_CATEGORY_STATIC(LogSubtitleSubsystem, Log, All);

void USubtitleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void USubtitleSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TrackCompleteTimer);
	}
	StopSubtitles();
	Super::Deinitialize();
}

void USubtitleSubsystem::PlaySubtitleTrack(
	const TArray<FCrankItSubtitleLine>& Lines,
	USoundBase* Voice,
	TFunction<void()> OnComplete)
{
	UWorld* World = GetWorld();
	if (!World || Lines.Num() == 0)
	{
		if (OnComplete)
		{
			OnComplete();
		}
		return;
	}

	TArray<FCrankItSubtitleCue> Cues;
	Cues.Reserve(Lines.Num());
	for (const FCrankItSubtitleLine& Line : Lines)
	{
		FCrankItSubtitleCue Cue;
		Cue.StartTimeSeconds = Line.StartTimeSeconds;
		Cue.EndTimeSeconds = Line.EndTimeSeconds;
		Cue.Text = FText::FromString(Line.Text);
		Cues.Add(Cue);
	}

	PendingTrackOnComplete = MoveTemp(OnComplete);
	World->GetTimerManager().ClearTimer(TrackCompleteTimer);

	if (Voice)
	{
		if (UCrankItAudioService* Audio = UCrankItAudioService::Get(this))
		{
			VoiceSoundHandle = Audio->Play2DTracked(Voice);
			if (UAudioComponent* VoiceComp = VoiceSoundHandle.AudioComponent.Get())
			{
				StartSubtitleTrack(Cues, VoiceComp);
				return;
			}
		}

		UE_LOG(LogSubtitleSubsystem, Warning, TEXT("PlaySubtitleTrack: 语音播放失败，改用世界时间轴。"));
	}

	StartSubtitleTrackWithWorldTime(Cues);

	if (!PendingTrackOnComplete)
	{
		return;
	}

	const float TrackEnd = Cues.Last().EndTimeSeconds + 0.05f;
	World->GetTimerManager().SetTimer(
		TrackCompleteTimer,
		FTimerDelegate::CreateUObject(this, &USubtitleSubsystem::FinishSubtitleTrack),
		TrackEnd,
		false);
}

void USubtitleSubsystem::FinishSubtitleTrack()
{
	TFunction<void()> Callback = MoveTemp(PendingTrackOnComplete);
	PendingTrackOnComplete = nullptr;
	StopSubtitles();
	if (Callback)
	{
		Callback();
	}
}

ETickableTickType USubtitleSubsystem::GetTickableTickType() const
{
	if (IsTemplate())
	{
		return ETickableTickType::Never;
	}
	return ETickableTickType::Always;
}

void USubtitleSubsystem::OnAudioPlaybackPercentNative(const UAudioComponent* InAudioComponent, const USoundWave* PlayingSoundWave, float PlaybackPercent)
{
	(void)PlayingSoundWave;
	if (InAudioComponent == SyncAudioWeak.Get())
	{
		CachedPlaybackPercent.store(PlaybackPercent, std::memory_order_relaxed);
	}
}

float USubtitleSubsystem::GetSyncedPlaybackSeconds(UAudioComponent* AudioComp) const
{
	if (!AudioComp || !AudioComp->IsPlaying())
	{
		return 0.f;
	}
	USoundBase* Sound = AudioComp->GetSound();
	if (!Sound)
	{
		return 0.f;
	}
	const float Duration = Sound->GetDuration();
	if (Duration <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}
	const float P = CachedPlaybackPercent.load(std::memory_order_relaxed);
	const float FromPercent = FMath::Clamp(P, 0.f, 1.f) * Duration;

	if (P > 0.001f)
	{
		return FromPercent;
	}

	if (const UWorld* World = GetWorld())
	{
		const float Elapsed = static_cast<float>(World->GetTimeSeconds() - TrackStartWorldTimeSeconds);
		return FMath::Clamp(Elapsed, 0.f, Duration);
	}

	return FromPercent;
}

void USubtitleSubsystem::StartSubtitleTrack(const TArray<FCrankItSubtitleCue>& Cues, UAudioComponent* SyncAudio)
{
	// 无组件则无法跟波形对齐，退化为世界时间轴，避免 Tick 里永远等不到有效时间。
	if (!SyncAudio)
	{
		UE_LOG(LogSubtitleSubsystem, Warning, TEXT("StartSubtitleTrack: SyncAudio 为空，改用世界时间轴。"));
		StartSubtitleTrackWithWorldTime(Cues);
		return;
	}

	TFunction<void()> SavedOnComplete = MoveTemp(PendingTrackOnComplete);
	StopSubtitles(SyncAudio);
	PendingTrackOnComplete = MoveTemp(SavedOnComplete);

	ActiveCues = Cues;
	ActiveCues.Sort([](const FCrankItSubtitleCue& A, const FCrankItSubtitleCue& B) {
		return A.StartTimeSeconds < B.StartTimeSeconds;
	});
	SyncAudioWeak = SyncAudio;
	bUseWorldTime = false;
	bTrackActive = ActiveCues.Num() > 0;
	CachedPlaybackPercent.store(0.f, std::memory_order_relaxed);
	if (const UWorld* World = GetWorld())
	{
		TrackStartWorldTimeSeconds = World->GetTimeSeconds();
	}
	if (UAudioComponent* A = SyncAudioWeak.Get())
	{
		A->OnAudioPlaybackPercentNative.AddUObject(this, &USubtitleSubsystem::OnAudioPlaybackPercentNative);
	}
}

void USubtitleSubsystem::StartSubtitleTrackWithWorldTime(const TArray<FCrankItSubtitleCue>& Cues)
{
	TFunction<void()> SavedOnComplete = MoveTemp(PendingTrackOnComplete);
	StopSubtitles();
	PendingTrackOnComplete = MoveTemp(SavedOnComplete);

	ActiveCues = Cues;
	ActiveCues.Sort([](const FCrankItSubtitleCue& A, const FCrankItSubtitleCue& B) {
		return A.StartTimeSeconds < B.StartTimeSeconds;
	});
	SyncAudioWeak.Reset();
	bUseWorldTime = true;
	if (const UWorld* World = GetWorld())
	{
		TrackStartWorldTimeSeconds = World->GetTimeSeconds();
	}
	else
	{
		TrackStartWorldTimeSeconds = 0.0;
	}
	bTrackActive = ActiveCues.Num() > 0;
}

void USubtitleSubsystem::StopSubtitles()
{
	StopSubtitles(nullptr);
}

void USubtitleSubsystem::StopSubtitles(UAudioComponent* PreserveVoiceComp)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TrackCompleteTimer);
	}
	PendingTrackOnComplete = nullptr;
	if (UAudioComponent* A = SyncAudioWeak.Get())
	{
		A->OnAudioPlaybackPercentNative.RemoveAll(this);
	}
	if (VoiceSoundHandle.IsValid())
	{
		UAudioComponent* VoiceComp = VoiceSoundHandle.AudioComponent.Get();
		if (!VoiceComp || VoiceComp != PreserveVoiceComp)
		{
			if (UCrankItAudioService* Audio = UCrankItAudioService::Get(this))
			{
				Audio->Stop(VoiceSoundHandle);
			}
			else
			{
				VoiceSoundHandle.AudioComponent.Reset();
				VoiceSoundHandle.WorldSource.Reset();
			}
		}
	}
	bTrackActive = false;
	ActiveCues.Reset();
	SyncAudioWeak.Reset();
	bUseWorldTime = false;
	CachedPlaybackPercent.store(0.f, std::memory_order_relaxed);
	SetCurrentSubtitleText(FText::GetEmpty());
}

void USubtitleSubsystem::SetCurrentSubtitleText(const FText& NewText)
{
	if (!NewText.EqualTo(LastBroadcastText))
	{
		LastBroadcastText = NewText;
		OnSubtitleLineChanged.Broadcast(NewText);
	}
}

int32 USubtitleSubsystem::FindCueIndexForTime(float T) const
{
	for (int32 i = 0; i < ActiveCues.Num(); ++i)
	{
		const FCrankItSubtitleCue& Cue = ActiveCues[i];
		if (T >= Cue.StartTimeSeconds && T < Cue.EndTimeSeconds)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

void USubtitleSubsystem::UpdateForTime(float PlaybackSeconds)
{
	// 间隙：没有 Cue 覆盖当前秒 → 广播空文本，由 UI 选择隐藏或保留底栏。
	const int32 Idx = FindCueIndexForTime(PlaybackSeconds);
	if (Idx != INDEX_NONE)
	{
		SetCurrentSubtitleText(ActiveCues[Idx].Text);
	}
	else
	{
		SetCurrentSubtitleText(FText::GetEmpty());
	}

	if (ActiveCues.Num() == 0)
	{
		return;
	}
	const float LastEnd = ActiveCues.Last().EndTimeSeconds;
	// 略大于 LastEnd 再收尾：避免浮点与帧边界导致最后一帧反复进出「已结束」状态。
	if (PlaybackSeconds >= LastEnd + 0.05f)
	{
		FinishSubtitleTrack();
	}
}

void USubtitleSubsystem::Tick(float DeltaTime)
{
	(void)DeltaTime;
	if (!bTrackActive || ActiveCues.Num() == 0)
	{
		return;
	}

	if (bUseWorldTime)
	{
		const UWorld* World = GetWorld();
		if (!World)
		{
			StopSubtitles();
			return;
		}
		const float Elapsed = static_cast<float>(World->GetTimeSeconds() - TrackStartWorldTimeSeconds);
		UpdateForTime(Elapsed);
		return;
	}

	UAudioComponent* Audio = SyncAudioWeak.Get();
	if (!Audio)
	{
		return;
	}
	// 语音自然播完：收尾并触发 OnComplete
	if (!Audio->IsPlaying())
	{
		FinishSubtitleTrack();
		return;
	}

	const float T = GetSyncedPlaybackSeconds(Audio);
	UpdateForTime(T);
}

TStatId USubtitleSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USubtitleSubsystem, STATGROUP_Tickables);
}

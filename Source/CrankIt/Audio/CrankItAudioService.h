#pragma once

// 统一音效播放：3D 经 ACrankItWorldSoundSource（可被 SoundDetector 探测），2D 为 UI/环境提示（不可探测）。
// 循环音由调用方在条件结束时 Stop(Handle)；单次音播完自动清理。

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrankItAudioService.generated.h"

class USoundBase;
class UAudioComponent;
class USceneComponent;
class ACrankItWorldSoundSource;

// 播放凭证：弱引用当前音频组件 / 3D 发声体，供 Stop 定点关闭
USTRUCT(BlueprintType)
struct CRANKIT_API FCrankItSoundHandle
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<UAudioComponent> AudioComponent;

	UPROPERTY()
	TWeakObjectPtr<ACrankItWorldSoundSource> WorldSource;

	// 组件或发声体仍有效时为 true
	bool IsValid() const;
};

UCLASS()
class CRANKIT_API UCrankItAudioService : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 从任意 WorldContext 取本世界的 AudioService
	static UCrankItAudioService* Get(const UObject* WorldContextObject);

	virtual void Deinitialize() override;

	// 单次 2D / UI 音效（不可被 SoundDetector 探测）
	UFUNCTION(BlueprintCallable, Category = "CrankIt|Audio")
	void Play2D(USoundBase* Sound, float VolumeMultiplier = 1.f);

	// 循环 2D 音效；条件结束时由调用方 Stop
	UFUNCTION(BlueprintCallable, Category = "CrankIt|Audio")
	FCrankItSoundHandle Play2DLoop(USoundBase* Sound, float VolumeMultiplier = 1.f);

	// 单次 2D，保留 UAudioComponent 句柄（供字幕同步等）；播完由调用方 Stop 或随 StopAll 清理
	UFUNCTION(BlueprintCallable, Category = "CrankIt|Audio")
	FCrankItSoundHandle Play2DTracked(USoundBase* Sound, float VolumeMultiplier = 1.f);

	// 在世界坐标播放 3D 音（可探测）；bLoop 为 true 时需手动 Stop
	UFUNCTION(BlueprintCallable, Category = "CrankIt|Audio")
	FCrankItSoundHandle PlayAtLocation3D(
		USoundBase* Sound,
		FVector Location,
		float VolumeMultiplier = 1.f,
		bool bLoop = false);

	// 附着到组件播放 3D 音（跟随移动，可探测）；bLoop 为 true 时需手动 Stop
	UFUNCTION(BlueprintCallable, Category = "CrankIt|Audio")
	FCrankItSoundHandle PlayAttached3D(
		USoundBase* Sound,
		USceneComponent* AttachTo,
		FName AttachSocketName = NAME_None,
		float VolumeMultiplier = 1.f,
		bool bLoop = false);

	// 停止指定句柄对应播放并清理组件 / 临时 Actor
	UFUNCTION(BlueprintCallable, Category = "CrankIt|Audio")
	void Stop(UPARAM(ref) FCrankItSoundHandle& Handle);

	// 停止本服务跟踪的全部活动播放
	UFUNCTION(BlueprintCallable, Category = "CrankIt|Audio")
	void StopAll();

	// 句柄指向的组件是否仍在播放
	UFUNCTION(BlueprintPure, Category = "CrankIt|Audio")
	bool IsPlaying(const FCrankItSoundHandle& Handle) const;

private:
	// 生成 ACrankItWorldSoundSource 并开始播放（可选附着）
	FCrankItSoundHandle SpawnWorldSource3D(
		USoundBase* Sound,
		const FVector& Location,
		USceneComponent* AttachTo,
		FName AttachSocketName,
		float VolumeMultiplier,
		bool bLoop);

	// 仅包装 2D 组件为句柄（无 WorldSource）
	FCrankItSoundHandle MakeHandleFromComponent(UAudioComponent* Component) const;

	// 记入 ActiveHandles，供 StopAll / 子系统销毁时清理
	void TrackHandle(const FCrankItSoundHandle& Handle);

	// 移除已失效的弱引用句柄
	void PruneFinishedHandles();

	// 当前由本服务创建、仍可能需要 Stop 的播放
	UPROPERTY()
	TArray<FCrankItSoundHandle> ActiveHandles;
};

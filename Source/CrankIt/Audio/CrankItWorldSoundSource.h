#pragma once

// 世界空间临时发声体：挂 UAudioComponent，供 SoundDetector 遍历探测。
// 由 UCrankItAudioService 生成；勿在关卡中手动摆放。

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrankItWorldSoundSource.generated.h"

class UAudioComponent;
class USoundBase;

UCLASS(NotPlaceable)
class CRANKIT_API ACrankItWorldSoundSource : public AActor
{
	GENERATED_BODY()

public:
	ACrankItWorldSoundSource();

	UAudioComponent* GetAudioComponent() const { return AudioComponent; }

	// 配置并播放；bLoop 为 false 时播完自动 Destroy
	void StartSound(USoundBase* Sound, float VolumeMultiplier, bool bLoop);

protected:
	// 单次播完 Destroy；循环则再次 Play
	UFUNCTION()
	void HandleAudioFinished();

	UPROPERTY(VisibleAnywhere, Category = "Audio")
	TObjectPtr<USceneComponent> RootComp;

	// 世界组件：SoundDetector 通过 Actor 遍历探测
	UPROPERTY(VisibleAnywhere, Category = "Audio")
	TObjectPtr<UAudioComponent> AudioComponent;

	// StartSound 传入的循环标志
	bool bLooping = false;
};

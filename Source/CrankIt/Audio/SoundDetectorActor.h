// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundSubmix.h"
#include "HAL/CriticalSection.h"
#include "AudioMixer.h"


#include "SoundDetectorActor.generated.h"

class USoundWaveformWidget;

UCLASS()
class CRANKIT_API ASoundDetectorActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASoundDetectorActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 探测器模型
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DetectorMesh;

	// 3D界面组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* ScreenWidget;

	// 音频组件用于检测声音
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAudioComponent* AudioComponent;

	/** 相对玩家摄像机位置的最大距离（厘米，与 UE 单位一致） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Detection")
	float DetectionRange = 1000.0f;

	/** 相对摄像机朝前的总锥角（度），半角为 DetectionAngle/2 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Detection")
	float DetectionAngle = 60.0f;

	// 更新频率（Hz）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Detection")
	float UpdateRate = 60.0f;

	UFUNCTION()
	void OnSubmixEnvelope(const TArray<float>& Envelope);
	
	UPROPERTY(EditAnywhere, Category="Audio")
	USoundSubmix* TargetSubmix;

	FOnSubmixEnvelopeBP EnvelopeDelegate;
	
	FCriticalSection EnvelopeMutex;
	/** Submix 包络回调写入；Tick 中拷贝后算 RMS，供 UI 波形/电平实时变化 */
	TArray<float> LatestEnvelope;

private:
	/**
	 * 遍历世界中正在播放的 UAudioComponent，按摄像机原点/朝前与 DetectionRange、DetectionAngle 求最大强度。
	 * PlaySoundAtLocation 等不落组件上的播放不在此列。返回值 [0,1]，无有效声源或未绑定玩家摄像机时为 0。
	 */
	float ComputeDirectionalSoundIntensity() const;

	/** 检测原点与朝向取自 APlayerCamera::CameraComp，而非本探测器 Actor */
	bool TryGetCameraDetectionFrame(FVector& OutOrigin, FVector& OutForward) const;

	/** Listener 为摄像机位置与朝前；与 ComputeDirectionalSoundIntensity 内锥判据一致 */
	float CalculateSoundIntensity(const FVector& ListenerOrigin, const FVector& ListenerForward,
		const FVector& SoundLocation, float SoundVolume) const;

	void UpdateWaveform(float SoundLevel);

	// 上次更新时间
	float LastUpdateTime;

	// 更新间隔
	float UpdateInterval;

	// Widget引用
	USoundWaveformWidget* WaveformWidget;
};

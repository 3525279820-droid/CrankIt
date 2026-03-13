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

	// 检测范围（米）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Detection")
	float DetectionRange = 1000.0f;

	// 检测角度（度）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Detection")
	float DetectionAngle = 60.0f;

	// 更新频率（Hz）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Detection")
	float UpdateRate = 60.0f;

	// 波形数据点数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Detection")
	int32 WaveformPoints = 100;

	UFUNCTION()
	void OnSubmixEnvelope(const TArray<float>& Envelope);
	
	UPROPERTY(EditAnywhere, Category="Audio")
	USoundSubmix* TargetSubmix;

	FOnSubmixEnvelopeBP EnvelopeDelegate;
	
	FCriticalSection EnvelopeMutex;
	TArray<float> LatestEnvelope; // 每通道最新包络

private:
	// 检测玩家前方的声音
	void DetectSoundInFront();

	// 计算声音强度
	float CalculateSoundIntensity(const FVector& SoundLocation, float SoundVolume);

	// 更新波形显示
	void UpdateWaveform(float SoundLevel);

	// 波形数据数组
	TArray<float> WaveformData;

	// 上次更新时间
	float LastUpdateTime;

	// 更新间隔
	float UpdateInterval;

	// Widget引用
	USoundWaveformWidget* WaveformWidget;
};

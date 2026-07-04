// Fill out your copyright notice in the Description page of Project Settings.

#include "SoundDetectorActor.h"
#include "SoundWaveformWidget.h"
#include "PlayerCamera.h"
#include "Kismet/GameplayStatics.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

// Sets default values
ASoundDetectorActor::ASoundDetectorActor()
{
 	// Set this actor to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	// 创建根组件
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// 创建探测器模型
	DetectorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DetectorMesh"));
	DetectorMesh->SetupAttachment(RootComponent);

	// 创建3D界面组件（挂在 Root 上，避免随 DetectorMesh 的非均匀缩放/变形影响 UI；位置与朝向请在蓝图里相对 Root 调整）
	ScreenWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ScreenWidget"));
	ScreenWidget->SetupAttachment(RootComponent);
	ScreenWidget->SetWidgetSpace(EWidgetSpace::World);
	// 按 Widget 的 DesiredSize 决定 RT 尺寸，避免窄条类布局被硬拉到固定 DrawSize 产生形变
	ScreenWidget->SetDrawAtDesiredSize(true);
	ScreenWidget->SetDrawSize(FVector2D(1920, 1080));
	ScreenWidget->SetPivot(FVector2D(0.5f, 0.5f));

	// 创建音频组件
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(DetectorMesh);
	AudioComponent->bAutoActivate = false;

	LastUpdateTime = 0.0f;
	UpdateRate = 60.0f;
	UpdateInterval = 1.0f / UpdateRate;
	WaveformWidget = nullptr;
}

// Called when the game starts or when spawned
void ASoundDetectorActor::BeginPlay()
{
	Super::BeginPlay();

	// 创建并设置Widget
	if (ScreenWidget)
	{
		UClass* WidgetClass = LoadClass<USoundWaveformWidget>(nullptr, TEXT("/Game/UI/SoundWaveformWidget.SoundWaveformWidget_C"));
		if (WidgetClass)
		{
			ScreenWidget->SetWidgetClass(WidgetClass);
			WaveformWidget = Cast<USoundWaveformWidget>(ScreenWidget->GetWidget());
		}
		else
		{
			// 如果找不到Widget类，创建一个默认的
			UE_LOG(LogTemp, Warning, TEXT("SoundWaveformWidget class not found. Please create it in Blueprint."));
		}
	}
	if (TargetSubmix)
	{
		EnvelopeDelegate.BindUFunction(this, FName("OnSubmixEnvelope"));
		// 用实例调用
		TargetSubmix->AddEnvelopeFollowerDelegate(GetWorld(), EnvelopeDelegate);
	}

}

void ASoundDetectorActor::OnSubmixEnvelope(const TArray<float>& Envelope)
{
	// 这个回调可能在音频线程被调用，尽量短且线程安全
	FScopeLock Lock(&EnvelopeMutex);
	LatestEnvelope = Envelope;
}


// Called every frame
void ASoundDetectorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LastUpdateTime += DeltaTime;
	if (LastUpdateTime >= UpdateInterval)
	{
		TArray<float> EnvelopeCopy;
		float EnvelopeRMS = 0.0f;
		{
			FScopeLock Lock(&EnvelopeMutex);
			EnvelopeCopy = LatestEnvelope;
		}

		if (EnvelopeCopy.Num() > 0)
		{
			float SumSq = 0.0f;
			for (float v : EnvelopeCopy)
			{
				SumSq += v * v;
			}
			EnvelopeRMS = FMath::Sqrt(SumSq / static_cast<float>(EnvelopeCopy.Num()));
		}

		// Submix 包络保留波形形状；乘摄像机锥方向增益，使 UI 随玩家视角而非探测器模型朝向变化
		const float DirectionalGain = ComputeDirectionalSoundIntensity();
		for (float& v : EnvelopeCopy)
		{
			v *= DirectionalGain;
		}

		if (WaveformWidget)
		{
			if (EnvelopeCopy.Num() > 0)
			{
				WaveformWidget->UpdateWaveform(EnvelopeCopy);
			}
			else
			{
				WaveformWidget->UpdateSoundLevel(0.0f);
			}
			WaveformWidget->UpdateSoundLevel(EnvelopeRMS * DirectionalGain);
		}

		LastUpdateTime = 0.0f;
	}
}

bool ASoundDetectorActor::TryGetCameraDetectionFrame(FVector& OutOrigin, FVector& OutForward) const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// 与 Monster 等逻辑一致：玩家 Pawn 为 APlayerCamera，检测帧用其 CameraComp（非探测器 Actor 位姿）
	const APlayerCamera* PlayerCam = Cast<APlayerCamera>(UGameplayStatics::GetPlayerPawn(World, 0));
	if (!PlayerCam || !PlayerCam->CameraComp)
	{
		return false;
	}

	OutOrigin = PlayerCam->CameraComp->GetComponentLocation();
	OutForward = PlayerCam->CameraComp->GetForwardVector();
	return true;
}

float ASoundDetectorActor::ComputeDirectionalSoundIntensity() const
{
	FVector ListenerOrigin;
	FVector ListenerForward;
	if (!TryGetCameraDetectionFrame(ListenerOrigin, ListenerForward))
	{
		return 0.0f;
	}

	float MaxSoundLevel = 0.0f;

	for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
	{
		AActor* Actor = *ActorIterator;
		if (!Actor)
		{
			continue;
		}
		TArray<UAudioComponent*> Components;
		Actor->GetComponents<UAudioComponent>(Components);

		for (UAudioComponent* AudioComp : Components)
		{
			if (AudioComp && AudioComp->IsPlaying())
			{
				const FVector SoundLocation = AudioComp->GetComponentLocation();
				const FVector ToSound = (SoundLocation - ListenerOrigin).GetSafeNormal();

				// 相对摄像机朝前的三维夹角（含俯仰），非水平面方位角
				const float SoundDot = FVector::DotProduct(ListenerForward, ToSound);
				if (SoundDot > 0.0f)
				{
					const float SoundDistance = FVector::Dist(ListenerOrigin, SoundLocation);
					if (SoundDistance <= DetectionRange)
					{
						const float AngleRad = FMath::Acos(FMath::Clamp(SoundDot, -1.0f, 1.0f));
						const float AngleDeg = FMath::RadiansToDegrees(AngleRad);

						if (AngleDeg <= DetectionAngle / 2.0f)
						{
							const float Volume = AudioComp->VolumeMultiplier;
							const float Intensity = CalculateSoundIntensity(
								ListenerOrigin, ListenerForward, SoundLocation, Volume);
							MaxSoundLevel = FMath::Max(MaxSoundLevel, Intensity);
						}
					}
				}
			}
		}
	}

	return MaxSoundLevel;
}

float ASoundDetectorActor::CalculateSoundIntensity(
	const FVector& ListenerOrigin,
	const FVector& ListenerForward,
	const FVector& SoundLocation,
	float SoundVolume) const
{
	const float Distance = FVector::Dist(ListenerOrigin, SoundLocation);

	// 距离衰减：以摄像机为原点
	float DistanceAttenuation = 1.0f / (1.0f + Distance * Distance / (DetectionRange * DetectionRange));

	const FVector ToSound = (SoundLocation - ListenerOrigin).GetSafeNormal();
	const float DotProduct = FVector::DotProduct(ListenerForward, ToSound);
	const float AngleFactor = FMath::Max(0.0f, DotProduct);

	const float Intensity = SoundVolume * DistanceAttenuation * AngleFactor;
	return FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void ASoundDetectorActor::UpdateWaveform(float SoundLevel)
{
	if (WaveformWidget)
	{
		WaveformWidget->UpdateSoundLevel(SoundLevel);
	}
}

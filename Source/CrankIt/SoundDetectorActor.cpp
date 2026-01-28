// Fill out your copyright notice in the Description page of Project Settings.

#include "SoundDetectorActor.h"
#include "SoundWaveformWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
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

	// 创建3D界面组件
	ScreenWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ScreenWidget"));
	ScreenWidget->SetupAttachment(DetectorMesh);
	ScreenWidget->SetWidgetSpace(EWidgetSpace::World);
	ScreenWidget->SetDrawSize(FVector2D(1920, 1080));
	ScreenWidget->SetPivot(FVector2D(0.5f, 0.5f));

	// 创建音频组件
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(DetectorMesh);
	AudioComponent->bAutoActivate = false;

	// 初始化波形数据
	WaveformPoints = 100;
	WaveformData.SetNum(WaveformPoints);
	for (int32 i = 0; i < WaveformPoints; i++)
	{
		WaveformData[i] = 0.0f;
	}

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
}

// Called every frame
void ASoundDetectorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 按更新频率检测声音
	LastUpdateTime += DeltaTime;
	if (LastUpdateTime >= UpdateInterval)
	{
		DetectSoundInFront();
		LastUpdateTime = 0.0f;
	}
}

void ASoundDetectorActor::DetectSoundInFront()
{
	FVector DetectorLocation = GetActorLocation();
	FVector DetectorForward = GetActorForwardVector();

	// 检测场景中的声音源
	float MaxSoundLevel = 0.0f;
	
	// 遍历所有Actor查找AudioComponent
	for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
	{
		AActor* Actor = *ActorIterator;
		if (!Actor) continue;
		TArray<UAudioComponent*> Components;
		Actor->GetComponents<UAudioComponent>(Components);
		
		for (UAudioComponent* AudioComp : Components)
		{
			if (AudioComp && AudioComp->IsPlaying())
			{
				FVector SoundLocation = AudioComp->GetComponentLocation();
				FVector ToSound = (SoundLocation - DetectorLocation).GetSafeNormal();
				
				// 检查声音是否在探测器前方
				float SoundDot = FVector::DotProduct(DetectorForward, ToSound);
				if (SoundDot > 0.0f) // 在前方
				{
					float SoundDistance = FVector::Dist(DetectorLocation, SoundLocation);
					if (SoundDistance <= DetectionRange)
					{
						// 检查角度是否在检测范围内
						float AngleRad = FMath::Acos(SoundDot);
						float AngleDeg = FMath::RadiansToDegrees(AngleRad);
						
						if (AngleDeg <= DetectionAngle / 2.0f)
						{
							// 计算声音强度（基于距离和音量）
							float Volume = AudioComp->VolumeMultiplier;
							float Intensity = CalculateSoundIntensity(SoundLocation, Volume);
							MaxSoundLevel = FMath::Max(MaxSoundLevel, Intensity);
						}
					}
				}
			}
		}
	}

	// 更新波形
	UpdateWaveform(MaxSoundLevel);
    // UE_LOG(LogTemp, Warning, TEXT("MaxSoundLevel: %f"), MaxSoundLevel);
}

float ASoundDetectorActor::CalculateSoundIntensity(const FVector& SoundLocation, float SoundVolume)
{
	FVector DetectorLocation = GetActorLocation();
	float Distance = FVector::Dist(DetectorLocation, SoundLocation);
	
	// 距离衰减（平方反比定律）
	float DistanceAttenuation = 1.0f / (1.0f + Distance * Distance / (DetectionRange * DetectionRange));
	
	// 角度衰减
	FVector DetectorForward = GetActorForwardVector();
	FVector ToSound = (SoundLocation - DetectorLocation).GetSafeNormal();
	float DotProduct = FVector::DotProduct(DetectorForward, ToSound);
	float AngleFactor = FMath::Max(0.0f, DotProduct); // 只考虑前方的声音
	
	// 综合计算
	float Intensity = SoundVolume * DistanceAttenuation * AngleFactor;
	return FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void ASoundDetectorActor::UpdateWaveform(float SoundLevel)
{
	// 将新数据添加到波形数组（滚动更新）
	if (WaveformData.Num() > 0)
	{
		// 移除第一个元素，添加新元素到末尾
		WaveformData.RemoveAt(0);
		WaveformData.Add(SoundLevel);
	}

	// 更新Widget显示
	if (WaveformWidget)
	{
		WaveformWidget->UpdateWaveform(WaveformData);
	}
}

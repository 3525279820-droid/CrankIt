#include "CrankItAudioService.h"

#include "CrankItWorldSoundSource.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

// 组件或 3D 发声体任一仍有效即视为句柄有效
bool FCrankItSoundHandle::IsValid() const
{
	return AudioComponent.IsValid() || WorldSource.IsValid();
}

// 从任意 WorldContext 取本世界的 AudioService
UCrankItAudioService* UCrankItAudioService::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}
	if (UWorld* World = WorldContextObject->GetWorld())
	{
		return World->GetSubsystem<UCrankItAudioService>();
	}
	return nullptr;
}

// 子系统销毁前停止全部活动播放，避免组件泄漏
void UCrankItAudioService::Deinitialize()
{
	StopAll();
	Super::Deinitialize();
}

// 单次 2D：PlaySound2D 不创建可遍历世界组件，SoundDetector 不会探测到
void UCrankItAudioService::Play2D(USoundBase* Sound, float VolumeMultiplier)
{
	if (!Sound)
	{
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	UGameplayStatics::PlaySound2D(World, Sound, VolumeMultiplier);
}

// 单次 2D：SpawnSound2D 保留组件句柄，供字幕等需要读取播放进度的逻辑使用
FCrankItSoundHandle UCrankItAudioService::Play2DTracked(USoundBase* Sound, float VolumeMultiplier)
{
	FCrankItSoundHandle Handle;
	if (!Sound)
	{
		return Handle;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return Handle;
	}

	UAudioComponent* Component = UGameplayStatics::SpawnSound2D(
		World,
		Sound,
		VolumeMultiplier,
		1.f,
		0.f,
		nullptr,
		false,
		false);

	if (!Component)
	{
		return Handle;
	}

	Component->bIsUISound = true;
	Component->bAllowSpatialization = false;

	Handle = MakeHandleFromComponent(Component);
	TrackHandle(Handle);
	return Handle;
}

// 循环 2D：持有 SpawnSound2D 组件以便 Stop；播完再 Play 以维持循环
FCrankItSoundHandle UCrankItAudioService::Play2DLoop(USoundBase* Sound, float VolumeMultiplier)
{
	FCrankItSoundHandle Handle;
	if (!Sound)
	{
		return Handle;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return Handle;
	}

	UAudioComponent* Component = UGameplayStatics::SpawnSound2D(
		World,
		Sound,
		VolumeMultiplier,
		1.f,
		0.f,
		nullptr,
		false,
		false);

	if (!Component)
	{
		return Handle;
	}

	Component->bIsUISound = true;
	Component->bAllowSpatialization = false;

	Component->OnAudioFinishedNative.AddLambda(
		[WeakComp = TWeakObjectPtr<UAudioComponent>(Component)](UAudioComponent* /*FinishedComp*/)
		{
			if (UAudioComponent* Comp = WeakComp.Get())
			{
				if (Comp->GetSound())
				{
					Comp->Play();
				}
			}
		});

	Handle = MakeHandleFromComponent(Component);
	TrackHandle(Handle);
	return Handle;
}

// 在世界坐标播放 3D 音（可探测）
FCrankItSoundHandle UCrankItAudioService::PlayAtLocation3D(
	USoundBase* Sound,
	FVector Location,
	float VolumeMultiplier,
	bool bLoop)
{
	return SpawnWorldSource3D(Sound, Location, nullptr, NAME_None, VolumeMultiplier, bLoop);
}

// 附着到组件播放 3D 音（跟随移动，可探测）
FCrankItSoundHandle UCrankItAudioService::PlayAttached3D(
	USoundBase* Sound,
	USceneComponent* AttachTo,
	FName AttachSocketName,
	float VolumeMultiplier,
	bool bLoop)
{
	const FVector Location = AttachTo ? AttachTo->GetComponentLocation() : FVector::ZeroVector;
	return SpawnWorldSource3D(Sound, Location, AttachTo, AttachSocketName, VolumeMultiplier, bLoop);
}

// 生成临时发声 Actor 并开始播放；可选附着到目标组件
FCrankItSoundHandle UCrankItAudioService::SpawnWorldSource3D(
	USoundBase* Sound,
	const FVector& Location,
	USceneComponent* AttachTo,
	FName AttachSocketName,
	float VolumeMultiplier,
	bool bLoop)
{
	FCrankItSoundHandle Handle;
	if (!Sound)
	{
		return Handle;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return Handle;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.ObjectFlags |= RF_Transient;

	ACrankItWorldSoundSource* Source = World->SpawnActor<ACrankItWorldSoundSource>(
		ACrankItWorldSoundSource::StaticClass(),
		Location,
		FRotator::ZeroRotator,
		SpawnParams);

	if (!Source)
	{
		return Handle;
	}

	if (AttachTo)
	{
		Source->AttachToComponent(
			AttachTo,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			AttachSocketName);
	}

	Source->StartSound(Sound, VolumeMultiplier, bLoop);

	Handle.WorldSource = Source;
	Handle.AudioComponent = Source->GetAudioComponent();
	TrackHandle(Handle);
	return Handle;
}

// 停止句柄对应播放：清回调、停组件，并 Destroy 3D 发声体或销毁 2D 组件
void UCrankItAudioService::Stop(FCrankItSoundHandle& Handle)
{
	const TWeakObjectPtr<UAudioComponent> WeakComp = Handle.AudioComponent;
	const TWeakObjectPtr<ACrankItWorldSoundSource> WeakSource = Handle.WorldSource;

	ActiveHandles.RemoveAll([&WeakComp, &WeakSource](const FCrankItSoundHandle& Entry)
	{
		return Entry.AudioComponent == WeakComp && Entry.WorldSource == WeakSource;
	});

	if (UAudioComponent* Component = WeakComp.Get())
	{
		Component->OnAudioFinishedNative.Clear();
		Component->Stop();
	}

	if (ACrankItWorldSoundSource* Source = WeakSource.Get())
	{
		Source->Destroy();
	}
	else if (UAudioComponent* Component = WeakComp.Get())
	{
		Component->DestroyComponent();
	}

	Handle.AudioComponent.Reset();
	Handle.WorldSource.Reset();
}

// 停止本服务跟踪的全部活动播放
void UCrankItAudioService::StopAll()
{
	PruneFinishedHandles();
	TArray<FCrankItSoundHandle> HandlesCopy = ActiveHandles;
	ActiveHandles.Empty();
	for (FCrankItSoundHandle& Handle : HandlesCopy)
	{
		Stop(Handle);
	}
}

// 句柄指向的组件是否仍在播放
bool UCrankItAudioService::IsPlaying(const FCrankItSoundHandle& Handle) const
{
	return Handle.AudioComponent.IsValid() && Handle.AudioComponent->IsPlaying();
}

// 仅包装 2D 组件为句柄（无 WorldSource）
FCrankItSoundHandle UCrankItAudioService::MakeHandleFromComponent(UAudioComponent* Component) const
{
	FCrankItSoundHandle Handle;
	Handle.AudioComponent = Component;
	return Handle;
}

// 记入 ActiveHandles，供 StopAll / 子系统销毁时清理
void UCrankItAudioService::TrackHandle(const FCrankItSoundHandle& Handle)
{
	PruneFinishedHandles();
	if (Handle.AudioComponent.IsValid() || Handle.WorldSource.IsValid())
	{
		ActiveHandles.Add(Handle);
	}
}

// 移除已失效的弱引用句柄
void UCrankItAudioService::PruneFinishedHandles()
{
	ActiveHandles.RemoveAll([](const FCrankItSoundHandle& Handle)
	{
		return !Handle.AudioComponent.IsValid() && !Handle.WorldSource.IsValid();
	});
}

#include "CrankItWorldSoundSource.h"

#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

ACrankItWorldSoundSource::ACrankItWorldSoundSource()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComp);
	AudioComponent->bAutoActivate = false;
	AudioComponent->bAutoDestroy = false;
}

// 配置空间化并播放；无效 Sound 时直接 Destroy
void ACrankItWorldSoundSource::StartSound(USoundBase* Sound, float VolumeMultiplier, bool bLoop)
{
	if (!AudioComponent || !Sound)
	{
		Destroy();
		return;
	}

	bLooping = bLoop;
	AudioComponent->SetSound(Sound);
	AudioComponent->SetVolumeMultiplier(VolumeMultiplier);
	AudioComponent->bIsUISound = false;
	AudioComponent->bAllowSpatialization = true;

	AudioComponent->OnAudioFinished.RemoveDynamic(this, &ACrankItWorldSoundSource::HandleAudioFinished);
	AudioComponent->OnAudioFinished.AddDynamic(this, &ACrankItWorldSoundSource::HandleAudioFinished);

	AudioComponent->Play();
}

// 资源本身已循环时通常不进入；否则循环再 Play，单次则 Destroy
void ACrankItWorldSoundSource::HandleAudioFinished()
{
	if (bLooping && AudioComponent && AudioComponent->GetSound())
	{
		AudioComponent->Play();
		return;
	}
	Destroy();
}

// Fill out your copyright notice in the Description page of Project Settings.

#include "DoubleAutoDoor.h"

#include "CrankItGameplaySubsystem.h"

ADoubleAutoDoor::ADoubleAutoDoor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootComp);

	LeftDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftDoor"));
	LeftDoorMesh->SetupAttachment(RootComp);

	RightDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightDoor"));
	RightDoorMesh->SetupAttachment(RootComp);
}

void ADoubleAutoDoor::BeginPlay()
{
	Super::BeginPlay();

	if (LeftDoorMesh)
	{
		LeftDoorStartRelative = LeftDoorMesh->GetRelativeLocation();
	}
	if (RightDoorMesh)
	{
		RightDoorStartRelative = RightDoorMesh->GetRelativeLocation();
	}
}

void ADoubleAutoDoor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopOpenSound();
	Super::EndPlay(EndPlayReason);
}

void ADoubleAutoDoor::DoorOpened()
{
	if (bHasOpened || bIsOpening)
	{
		return;
	}

	bIsOpening = true;
	OpenElapsed = 0.f;
	StartOpenSound();
	SetActorTickEnabled(true);
}

void ADoubleAutoDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsOpening)
	{
		return;
	}

	OpenElapsed += DeltaTime;
	const float Duration = FMath::Max(OpenDuration, KINDA_SMALL_NUMBER);
	const float Alpha = FMath::Clamp(OpenElapsed / Duration, 0.f, 1.f);
	UpdateDoorPositions(Alpha);

	if (Alpha >= 1.f)
	{
		FinishOpening();
	}
}

void ADoubleAutoDoor::UpdateDoorPositions(float Alpha)
{
	if (LeftDoorMesh)
	{
		LeftDoorMesh->SetRelativeLocation(FMath::Lerp(LeftDoorStartRelative, LeftDoorStartRelative + LeftDoorOpenOffset, Alpha));
	}
	if (RightDoorMesh)
	{
		RightDoorMesh->SetRelativeLocation(FMath::Lerp(RightDoorStartRelative, RightDoorStartRelative + RightDoorOpenOffset, Alpha));
	}
}

void ADoubleAutoDoor::StartOpenSound()
{
	if (!OpenSound)
	{
		return;
	}

	if (UCrankItAudioService* Audio = UCrankItAudioService::Get(this))
	{
		StopOpenSound();
		OpenSoundHandle = Audio->PlayAttached3D(OpenSound, RootComp, NAME_None, 1.f, true);
	}
}

void ADoubleAutoDoor::StopOpenSound()
{
	if (!OpenSoundHandle.IsValid())
	{
		return;
	}

	if (UCrankItAudioService* Audio = UCrankItAudioService::Get(this))
	{
		Audio->Stop(OpenSoundHandle);
	}
	else
	{
		OpenSoundHandle = FCrankItSoundHandle();
	}
}

void ADoubleAutoDoor::FinishOpening()
{
	bIsOpening = false;
	bHasOpened = true;
	SetActorTickEnabled(false);
	StopOpenSound();

	// 经 GameplaySubsystem 解锁北向生成，避免门直接依赖 Monster
	if (UWorld* World = GetWorld())
	{
		if (UCrankItGameplaySubsystem* Gameplay = World->GetSubsystem<UCrankItGameplaySubsystem>())
		{
			Gameplay->NotifyNorthEntryDoorOpened();
		}
	}
}

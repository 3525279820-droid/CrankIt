// Fill out your copyright notice in the Description page of Project Settings.

#include "DoubleAutoDoor.h"

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

void ADoubleAutoDoor::DoorOpened()
{
	if (bHasOpened || bIsOpening)
	{
		return;
	}

	bIsOpening = true;
	OpenElapsed = 0.f;
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
		bIsOpening = false;
		bHasOpened = true;
		SetActorTickEnabled(false);
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

// Fill out your copyright notice in the Description page of Project Settings.


#include "BgCamera.h"



// Sets default values
ABgCamera::ABgCamera()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>("SpringArmComp");
	SetRootComponent(SpringArmComp);
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>("BackgroundCamera");
	CameraComp->SetupAttachment(SpringArmComp);

	LightComponent = CreateDefaultSubobject<UPointLightComponent>("LightComp");
	LightComponent->SetupAttachment(CameraComp);
}

void ABgCamera::ResetPosition()
{
	FVector CameraLocation = SpringArmComp->GetComponentLocation();
	CameraLocation.Z = -1300.f;
	SpringArmComp->SetWorldLocation(CameraLocation);
}

// Called when the game starts or when spawned
void ABgCamera::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABgCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// FVector CameraLocation = SpringArmComp->GetComponentLocation();
	// SpringArmComp->AddLocalOffset(FVector(0, 0 , DeltaTime * -500.f));
	// if(2810.f -  FMath::Abs(CameraLocation.Z)< KINDA_SMALL_NUMBER)
	// {
	// 	ResetPosition();
	// }
}


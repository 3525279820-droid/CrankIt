// Fill out your copyright notice in the Description page of Project Settings.


#include "ComputerScreenActor.h"

// Sets default values
AComputerScreenActor::AComputerScreenActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ScreenMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScreenMesh"));
	SetRootComponent(ScreenMesh);

	ScreenWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ScreenWidget"));
	ScreenWidget->SetupAttachment(ScreenMesh);

	
	ScreenWidget->SetWidgetSpace(EWidgetSpace::World);
	ScreenWidget->SetDrawSize(FVector2D(1024, 768));
	ScreenWidget->SetPivot(FVector2D(0.5f, 0.5f));
}

// Called when the game starts or when spawned
void AComputerScreenActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AComputerScreenActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


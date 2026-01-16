// Fill out your copyright notice in the Description page of Project Settings.


#include "ComputerScreenActor.h"

#include "PlayerCamera.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"

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
	ScreenWidget->SetDrawSize(FVector2D(1920, 1080));
	ScreenWidget->SetPivot(FVector2D(0.5f, 0.5f));

	ScreenMesh->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void AComputerScreenActor::BeginPlay()
{
	Super::BeginPlay();
}

void AComputerScreenActor::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);

	ACameraActor* FixedCamera = Cast<ACameraActor>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ACameraActor::StaticClass())
	);
		
	// 切换摄像机
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && FixedCamera)
	{
		// 保存原始视角
		APawn* Pawn = PC->GetPawn();
		APlayerCamera* PCamera = Cast<APlayerCamera>(Pawn);
		if(PCamera)
		{
			PCamera->OriginalViewTarget = PC->GetViewTarget();
		}
		
		PC->SetViewTargetWithBlend(FixedCamera, .5f); // 平滑切换
	}
}

// Called every frame
void AComputerScreenActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


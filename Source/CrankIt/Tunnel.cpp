// Fill out your copyright notice in the Description page of Project Settings.


#include "Tunnel.h"

// Sets default values
ATunnel::ATunnel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root Comp"));
	SetRootComponent(RootComp);

	Tunnel_1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tunnel 1"));
	Tunnel_1->SetupAttachment(RootComp);

	Tunnel_2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tunnel 2"));
	Tunnel_2->SetupAttachment(RootComp);
	

}

// Called when the game starts or when spawned
void ATunnel::BeginPlay()
{
	Super::BeginPlay();
	T1Location = Tunnel_1->GetComponentLocation();
	T2Location = Tunnel_2->GetComponentLocation();

	FBoxSphereBounds Bounds = Tunnel_2->Bounds;
	FVector Extent = Bounds.BoxExtent; // 半尺寸（X/Y/Z方向）
	
	MoveDistance = Extent.Z * 2;

	startLocation = T2Location;
	startLocation.Z += MoveDistance;
	TargetLocation = FVector(0, 0, startLocation.Z+MoveDistance);

}

// Called every frame
void ATunnel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Move(DeltaTime, Tunnel_1);
	Move(DeltaTime, Tunnel_2);

}

void ATunnel::Move(float DeltaTime, UStaticMeshComponent* MovePart)
{
	FVector CurrentLocation = MovePart->GetComponentLocation();
	float Speed = MoveDistance / MoveTime;

	// 如果还没到目标，继续插值
	if (!FMath::IsNearlyEqual(CurrentLocation.Z, TargetLocation.Z, .5f))
	{
		float NewZ = FMath::FInterpConstantTo(CurrentLocation.Z, TargetLocation.Z, DeltaTime, Speed);
		// float NewZ = CurrentLocation.Z + 10.f;
		NewLocation = FVector(CurrentLocation.X, CurrentLocation.Y, NewZ);
		MovePart->SetWorldLocation(NewLocation);
		// MovePart->AddWorldOffset(FVector(0, 0, 10.f));
		UE_LOG(LogTemp, Display, TEXT("X: %f, Y: %f, Z: %f"), T2Location.X, T2Location.Y, T2Location.Z)

	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Arrived"))

		// 到达后跳转到指定位置，并更新新的目标
		if(MovePart==Tunnel_1)
		{
			UE_LOG(LogTemp, Display, TEXT("T1 Arrived"))
			FBoxSphereBounds Bounds = Tunnel_2->Bounds;
			FVector Origin = Bounds.Origin;   // 包围盒中心点
			FVector Extent = Bounds.BoxExtent; // 半尺寸（X/Y/Z方向）
		
			// 底端位置（世界坐标）
			FVector Bottom = Origin - FVector(0, 0, Extent.Z) * 2.65;
			Bottom.X = T2Location.X;
			Bottom.Y = T2Location.Y;
			
			MovePart->SetWorldLocation(Bottom);
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("T2 Arrived"))
			FBoxSphereBounds Bounds = Tunnel_1->Bounds;
			FVector Origin = Bounds.Origin;   // 包围盒中心点
			FVector Extent = Bounds.BoxExtent; // 半尺寸（X/Y/Z方向）
		
			// 底端位置（世界坐标）
			FVector Bottom = Origin - FVector(0, 0, Extent.Z) * 2.65;
			Bottom.X = T1Location.X;
			Bottom.Y = T1Location.Y;
			MovePart->SetWorldLocation(Bottom);
		}
	}
}

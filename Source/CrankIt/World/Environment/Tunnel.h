// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tunnel.generated.h"

UCLASS()
class CRANKIT_API ATunnel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATunnel();
	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootComp;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Tunnel_1;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Tunnel_2;


	FVector T1Location;
	FVector T2Location;
	
	float MoveDistance;

	UPROPERTY(EditAnywhere)
	float MoveTime = 0.4;

	UPROPERTY(EditAnywhere, Category="间隔距离校准")
	float DistanceMultiple = 2.65;
	
	FVector TargetLocation;
	FVector startLocation;
	FVector NewLocation;

	UPROPERTY(EditAnywhere)
	bool bShouldMove = true;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Move(float DeltaTime, UStaticMeshComponent* MovePart);

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DoubleAutoDoor.generated.h"

UCLASS(Blueprintable)
class CRANKIT_API ADoubleAutoDoor : public AActor
{
	GENERATED_BODY()

public:
	ADoubleAutoDoor();

	UFUNCTION(BlueprintCallable, Category = "Door")
	void DoorOpened();

	UPROPERTY(VisibleAnywhere, Category = "Door")
	USceneComponent* RootComp;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	UStaticMeshComponent* LeftDoorMesh;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	UStaticMeshComponent* RightDoorMesh;

	/** 开门动画时长*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door", meta = (ClampMin = "0.01"))
	float OpenDuration = 10.f;


	// 左偏移量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	FVector LeftDoorOpenOffset = FVector(-100.f, 0.f, 0.f);

	// 右偏移量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	FVector RightDoorOpenOffset = FVector(100.f, 0.f, 0.f);

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	FVector LeftDoorStartRelative;
	FVector RightDoorStartRelative;

	float OpenElapsed = 0.f;
	bool bIsOpening = false;
	bool bHasOpened = false;

	void UpdateDoorPositions(float Alpha);
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Actor.h"
#include "TerminalWidget.h"
#include "ComputerScreenActor.generated.h"

UCLASS()
class CRANKIT_API AComputerScreenActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AComputerScreenActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void NotifyActorOnClicked(FKey ButtonPressed) override;

	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ScreenMesh;

	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* ScreenWidget;

};

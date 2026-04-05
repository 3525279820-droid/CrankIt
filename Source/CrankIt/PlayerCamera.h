// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MineConsole.h"
#include "ComputerScreenActor.h"

#include "PlayerCamera.generated.h"

UCLASS()
class CRANKIT_API APlayerCamera : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerCamera();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void InteractInput(const FInputActionValue& InputActionValue);
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void TurnInput(const FInputActionValue& value);

	void ExitScreenInput(const FInputActionValue& value);

	void PickBattery();
	
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArmComp;
	
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* BatteryHoldPoint;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* TurnAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* IntereactAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ExitScreen;
	
	UPROPERTY()
	AActor* OriginalViewTarget;
	
	FRotator DeltaRotation = FRotator::ZeroRotator;

	APlayerController* PlayerController;

	APlayerController* PlayerControllerRef;

	AMineConsole* MineConsole;

	ABattery* TargetBattery;

	ABattery* HoldBattery;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Actor.h"
#include "TerminalWidget.h"
#include "ComputerScreenActor.generated.h"

class UTerminalWidget;

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

	FTimerHandle LiftDesendTimerHandle;

	void SetBeginText();

	void SetFirstPromptText();

	UPROPERTY(EditAnywhere)
	float DesendTime = 3.f;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ScreenMesh;

	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* ScreenWidget;

	UPROPERTY(VisibleAnywhere)
	ACameraActor* FixedCamera;

	UPROPERTY(VisibleAnywhere)
	APlayerController* PC;

	// 终端小部件引用，用于控制键盘焦点
	UPROPERTY()
	UTerminalWidget* TerminalWidget;

	// 记录上一帧是否处于电脑屏幕视角，用于检测状态切换
	bool bWasInComputerView = false;

};

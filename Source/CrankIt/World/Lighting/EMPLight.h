// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SpotLightComponent.h"

#include "EMPLight.generated.h"

UCLASS()
class CRANKIT_API AEMPLight : public AActor
{
	GENERATED_BODY()
	
public:	
	AEMPLight();
	virtual void Tick(float DeltaTime) override;

	void ResetLight(float DeltaTime);

	/** 播放按钮按下并回弹动画（点击触发时调用） */
	UFUNCTION(BlueprintCallable, Category = "Button")
	void PlayButtonPress();
	
	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootComp;
	
	UPROPERTY(VisibleAnywhere)
	USpotLightComponent* EMPLightLeft;

	UPROPERTY(VisibleAnywhere)
	USpotLightComponent* EMPLightRight;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ButtonMesh;

	UPROPERTY(EditAnywhere)
	float LightIntensity = 0.f;

	/** 按下时在本地 Z 轴下移的距离（厘米） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button")
	float ButtonPressDistance = 2.f;

	/** 按下阶段时长（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button")
	float ButtonPressDownDuration = 0.08f;

	/** 回弹阶段时长（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button")
	float ButtonPressReleaseDuration = 0.12f;

	FTimerHandle ResetLightHandle;
	
protected:
	virtual void BeginPlay() override;

private:
	enum class EButtonPressPhase : uint8
	{
		Idle,
		Pressing,
		Releasing
	};

	void UpdateButtonPress(float DeltaTime);

	FVector ButtonRestRelativeLocation = FVector::ZeroVector;
	float ButtonPressAlpha = 0.f;
	EButtonPressPhase ButtonPressPhase = EButtonPressPhase::Idle;
};

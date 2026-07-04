// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Battery.h"
#include "BatterySlotTrigger.generated.h"
/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CRANKIT_API UBatterySlotTrigger : public UBoxComponent
{
	GENERATED_BODY()
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/** 开局时已在盒体内的电池不会收到 BeginOverlap，据此同步充电状态；下一帧再跑一次以应对首帧碰撞未就绪。 */
	void SyncOverlappingBatteryChargeState();

public:
	// Called every frame
	UBatterySlotTrigger();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** called when something leaves the sphere component */
	UFUNCTION()
	void OnButtonClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp,  AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** 放回槽位时线性插值：Alpha 每秒增加量（1 ≈ 约 1 秒从起点到槽位） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battery|Placement")
	float BatteryReturnLerpSpeed = 2.f;

protected:
	void UpdateBatteryReturnMotion(float DeltaTime);

	UPROPERTY(Transient)
	ABattery* ReturningBattery = nullptr;

	FVector BatteryReturnStartLoc = FVector::ZeroVector;
	FQuat BatteryReturnStartQuat = FQuat::Identity;
	float BatteryReturnAlpha = 0.f;
};

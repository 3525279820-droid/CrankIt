// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "Battery.h"
#include "BatterySlotTrigger.h"
#include "MineConsole.generated.h"

class UPointLightComponent;
UCLASS()
class CRANKIT_API AMineConsole : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMineConsole();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetShouldRotate(bool CanRotate);

	void StartLightingOffSequence();

	void LightNext();

	void LightOff();

	void AllLightsOff();

	void ChargeBattery();

	void CheckNeedCharge();

	/** 玩家在 SkipTutorial 中点击 Yes 后为 true，不再显示教程字幕。 */
	UPROPERTY(BlueprintReadWrite, Category = "Tutorial")
	bool bSkipedTutorial = false;

	/** 充电教程字幕已全部展示过（充满 3 格后），不再重复播放。 */
	UPROPERTY(BlueprintReadWrite, Category = "Tutorial")
	bool bBatteryFirstCharged = false;

	/** 方向 3 下已首次触发 LightTrigger 并播放过 EMP 教程字幕。 */
	UPROPERTY(BlueprintReadWrite, Category = "Tutorial")
	bool bEMPFirstTriggered = false;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootComp;
	
	// UPROPERTY(VisibleAnywhere)
	// UStaticMeshComponent* ConsoleBase;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ChargeHandle;

	UPROPERTY(VisibleAnywhere)
	TArray<UPointLightComponent*> ChargeLights;

	UPROPERTY(VisibleAnywhere)
	TArray<UBatterySlotTrigger*> BatterySlots;


	
	FTimerHandle ChargeLightTimer;

	int32 CurrentLightIndex = 0;

	ABattery* Battery;

	TArray<ABattery*> Batteries;

	

	bool ShouldRotate = false;
	
	float AngularVelocityYaw = 0.f;

	UPROPERTY(EditAnywhere)
	float SpinVelocity = 800.f;

	void TryShowChargeTutorialSubtitle(int32 NewChargeLevel);

	/** 方向 3 下首次触发 LightTrigger 时播放 EMP 教程字幕轨。 */
	void TryShowLightTutorialSubtitle();

	/** 已展示过的最高充电教程行（1~3），避免多块电池重复播同一句。 */
	int32 ChargeTutorialLineShownUpTo = 0;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



};

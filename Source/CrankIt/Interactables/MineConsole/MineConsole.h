// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "Battery.h"
#include "BatterySlotTrigger.h"
#include "CrankItAudioService.h"
#include "MineConsole.generated.h"

class UPointLightComponent;
class USoundBase;
UCLASS()
class CRANKIT_API AMineConsole : public AActor
{
	GENERATED_BODY()
	
public:
	AMineConsole();
	virtual void Tick(float DeltaTime) override;

	void SetShouldRotate(bool CanRotate);

	void StartLightingOffSequence();

	void LightNext();

	void LightOff();

	void AllLightsOff();

	void ChargeBattery();

	void CheckNeedCharge();

	// 充电教程字幕已全部展示过（充满 3 格后），不再重复播放
	UPROPERTY(BlueprintReadWrite, Category = "Tutorial")
	bool bBatteryFirstCharged = false;

	// 方向 3 下已首次触发 LightTrigger 并播放过 EMP 教程字幕
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

	float AngularVelocityYaw = 0.f;

	UPROPERTY(EditAnywhere)
	float SpinVelocity = 800.f;

	UPROPERTY(EditAnywhere, Category = "MineConsole|Audio")
	TObjectPtr<USoundBase> CrankLoopSound = nullptr;

	void TryShowChargeTutorialSubtitle(int32 NewChargeLevel);

	// 标记 Skip 教程已选 Yes；外部请优先走 UCrankItGameplaySubsystem::SetTutorialSkipped
	void SetTutorialSkipped(bool bSkipped);

	UFUNCTION(BlueprintPure, Category = "Tutorial")
	bool IsTutorialSkipped() const { return bSkipedTutorial; }

	// 是否应在当前朝向下播放 EMP 教程（West=3，与 IntroFlow::SkipTutorialDirectionIndex 一致）
	bool ShouldShowEMPTutorial(int32 PlayerDirectionIndex) const;

	// 方向 3 下首次触发 LightTrigger 时播放 EMP 教程字幕轨
	void TryShowLightTutorialSubtitle();

	// 触发 EMP 教程所需的玩家朝向索引（Directions 中 West）
	static constexpr int32 EMPTutorialDirectionIndex = 3;

	// 已展示过的最高充电教程行（1~3），避免多块电池重复播同一句
	int32 ChargeTutorialLineShownUpTo = 0;

protected:
	virtual void BeginPlay() override;

	// 玩家在 SkipTutorial 中选 Yes 后为 true；C++ 外部请用 SetTutorialSkipped / IsTutorialSkipped
	UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
	bool bSkipedTutorial = false;

private:
	/** 玩家悬停 ChargeHandle 时为 true；外部请用 SetShouldRotate */
	bool ShouldRotate = false;

	FCrankItSoundHandle CrankLoopSoundHandle;
};

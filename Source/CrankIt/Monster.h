// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "PlayerCamera.h"
#include "Monster.generated.h"

UCLASS()
class CRANKIT_API AMonster : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMonster();

protected:
	virtual void BeginPlay() override;

	// 怪物前进次数
	int32 AdvanceCount;

	// 定时器
	FTimerHandle AdvanceTimerHandle;

	// 玩家引用
	APlayerCamera* PlayerActor;

	// 前进间隔时间
	UPROPERTY(EditAnywhere, Category="Monster")
	float AdvanceInterval = 5.f;

	// 前进距离
	UPROPERTY(EditAnywhere, Category="Monster")
	float AdvanceStep = 200.f;

	// 最大前进次数
	UPROPERTY(EditAnywhere, Category="Monster")
	int32 MaxAdvanceCount = 3;
	
	// 是否当前活跃（可见、可驱赶）
	UPROPERTY(VisibleAnywhere, Category="Monster")
	bool bIsActive=true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Monster | Audio")
	TObjectPtr<USoundBase> MonsterAppearSound;

	/** 出现音效经此组件播放，便于 SoundDetector 按空间与朝向检测 */
	UPROPERTY(VisibleAnywhere, Category="Monster | Audio")
	TObjectPtr<UAudioComponent> AppearAudioComponent;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MonsterBase;
	
	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootComp;
	// 前进逻辑
	void AdvanceTowardsPlayer();

public:
	// 怪物当前方向（东、西、北）
	UPROPERTY(VisibleAnywhere, Category="Monster")
	FString CurrentDirection;
	
	// 初始化怪物位置
	void SpawnAtRandomDirection(APlayerCamera* Player);

	// 被驱赶逻辑（灯开关调用）
	void Repel();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

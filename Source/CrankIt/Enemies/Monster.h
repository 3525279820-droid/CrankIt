#pragma once

// 三向逼近怪物：EMP 灯驱赶可重置；触达上限后经 GameplaySubsystem 触发 JumpScare

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Monster.generated.h"

class APlayerCamera;

UCLASS()
class CRANKIT_API AMonster : public AActor
{
	GENERATED_BODY()
	
public:	
	AMonster();

protected:
	virtual void BeginPlay() override;

	// 当前连续逼近次数；超过 MaxAdvanceCount 即游戏失败
	int32 AdvanceCount;

	// 周期逼近定时器
	FTimerHandle AdvanceTimerHandle;

	// 玩家 Pawn（APlayerCamera）
	APlayerCamera* PlayerActor;

	// 两次逼近之间的间隔（秒）
	UPROPERTY(EditAnywhere, Category="Monster")
	float AdvanceInterval = 5.f;

	// 单次逼近沿玩家方向移动的距离（厘米）
	UPROPERTY(EditAnywhere, Category="Monster")
	float AdvanceStep = 200.f;

	// 允许的最大逼近次数；再一次即触发失败跳杀
	UPROPERTY(EditAnywhere, Category="Monster")
	int32 MaxAdvanceCount = 3;

	// 被驱赶后重新生成的随机延迟下限（秒）
	UPROPERTY(EditAnywhere, Category="Monster", meta=(ClampMin="0.0"))
	float RespawnDelayMin = 2.f;

	// 被驱赶后重新生成的随机延迟上限（秒）
	UPROPERTY(EditAnywhere, Category="Monster", meta=(ClampMin="0.0"))
	float RespawnDelayMax = 6.f;

	// 生成时相对玩家的水平距离（厘米）；东/西走 Y，北走 X
	UPROPERTY(EditAnywhere, Category="Monster", meta=(ClampMin="0.0"))
	float SpawnDistance = 500.f;
	
	// 是否当前活跃（可见、可被灯光驱赶）
	UPROPERTY(VisibleAnywhere, Category="Monster")
	bool bIsActive=true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Monster | Audio")
	TObjectPtr<USoundBase> MonsterAppearSound;

	// 出现音效经此组件播放，便于 SoundDetector 按空间与朝向检测
	UPROPERTY(VisibleAnywhere, Category="Monster | Audio")
	TObjectPtr<UAudioComponent> AppearAudioComponent;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MonsterBase;
	
	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootComp;

	// 定时逼近；触达上限时转交 UCrankItGameplaySubsystem::TriggerGameOverJumpScare
	void AdvanceTowardsPlayer();

	bool bIsSpawned = false;

	// C++ 外部启用怪物生成请调用 EnableSpawning，勿直接写 bSpawnable
	UPROPERTY(BlueprintReadOnly, Category = "Monster")
	bool bSpawnable = false;

public:
	void EnableSpawning() { bSpawnable = true; }

	UFUNCTION(BlueprintPure, Category = "Monster")
	bool IsSpawningEnabled() const { return bSpawnable; }

	// 当前生成方位（East / West / North）
	UPROPERTY(VisibleAnywhere, Category="Monster")
	FString CurrentDirection;
	
	// 在玩家东/西/北随机方位生成并播出现音
	void SpawnAtRandomDirection(APlayerCamera* Player);

	// 被灯光驱赶：清定时器并延迟重新生成
	void Repel();

	virtual void Tick(float DeltaTime) override;

	bool bTimerStarted = false;

	// 失败跳杀准备：停生成/前进、移到 Pose（GameOverMonsterLocation）、显示网格体
	void BeginGameOverJumpScare(const FTransform& Pose);

};

// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
AMonster::AMonster()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AdvanceCount = 0;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root Comp"));
	SetRootComponent(RootComp);
	
	MonsterBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MonsterBase"));
	MonsterBase->SetupAttachment(RootComp);
}

// Called when the game starts or when spawned
void AMonster::BeginPlay()
{
	Super::BeginPlay();
	PlayerActor = Cast<APlayerCamera>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	// 随机生成位置
	SpawnAtRandomDirection(PlayerActor);

	// 启动定时器
	GetWorldTimerManager().SetTimer(AdvanceTimerHandle, this, &AMonster::AdvanceTowardsPlayer, AdvanceInterval, true);
}
void AMonster::SpawnAtRandomDirection(APlayerCamera* Player)
{
	if (!Player) return;

	FVector PlayerLocation = Player->GetActorLocation();
	TArray<FString> Directions = { "East", "West", "North" };
	CurrentDirection = Directions[FMath::RandRange(0, Directions.Num() - 1)];
	bIsActive = true;
	FVector SpawnLocation = PlayerLocation;
	if (CurrentDirection == "East") SpawnLocation += FVector(0.f, 500.f, 100.f);
	else if (CurrentDirection == "West") SpawnLocation += FVector(0.f, -500.f, 100.f);
	else if (CurrentDirection == "North") SpawnLocation += FVector(500.f, 0.f, 100.f);
	UE_LOG(LogTemp, Display, TEXT("怪物生成在：%s"), *CurrentDirection)

	SetActorLocation(SpawnLocation);
}

void AMonster::AdvanceTowardsPlayer()
{
	AdvanceCount++;

	if (AdvanceCount > MaxAdvanceCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("Game Over! Monster reached the player."));
		GetWorldTimerManager().ClearTimer(AdvanceTimerHandle);
		return;
	}

	// 怪物瞬移靠近玩家
	FVector Direction = (PlayerActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	SetActorLocation(GetActorLocation() + Direction * AdvanceStep);

	UE_LOG(LogTemp, Display, TEXT("Monster advanced. Count: %d"), AdvanceCount);
}

void AMonster::Repel()
{
	if (!bIsActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("No active monster to repel."));
		return;
	}
	UE_LOG(LogTemp, Display, TEXT("Monster repelled by light!"));
	// 清除前进定时器
	bIsActive=false;
	GetWorldTimerManager().ClearTimer(AdvanceTimerHandle);
	
	// 重置前进次数
	AdvanceCount = 0;

	// 随机等待时间
	float Delay = FMath::RandRange(2.f, 6.f);
	UE_LOG(LogTemp, Display, TEXT("Monster respawn in %f"), Delay);

	// 设置延迟生成定时器
	FTimerHandle RespawnHandle;
	GetWorldTimerManager().SetTimer(RespawnHandle, [this]()
	{
		SpawnAtRandomDirection(PlayerActor);
		// 重新启动前进定时器
		GetWorldTimerManager().SetTimer(AdvanceTimerHandle, this, &AMonster::AdvanceTowardsPlayer, AdvanceInterval, true);
	}, Delay, false);
}
// Called every frame
void AMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(!bIsActive && MonsterBase->IsVisible())
	{
		MonsterBase->SetVisibility(false);
	}
	else if(bIsActive && !MonsterBase->IsVisible())
	{
		MonsterBase->SetVisibility(true);
	}

}


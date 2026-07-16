#include "Monster.h"

#include "PlayerCamera.h"
#include "CrankItGameplaySubsystem.h"
#include "Kismet/GameplayStatics.h"

AMonster::AMonster()
{
	PrimaryActorTick.bCanEverTick = true;
	AdvanceCount = 0;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root Comp"));
	SetRootComponent(RootComp);
	
	MonsterBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MonsterBase"));
	MonsterBase->SetupAttachment(RootComp);

	// 出现音用组件播放而非 PlaySoundAtLocation，以便 SoundDetector 遍历 UAudioComponent + 摄像机锥检测
	AppearAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AppearAudioComponent"));
	AppearAudioComponent->SetupAttachment(RootComp);
	AppearAudioComponent->bAutoActivate = false;
}

void AMonster::BeginPlay()
{
	Super::BeginPlay();
	PlayerActor = Cast<APlayerCamera>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	MonsterBase->SetVisibility(false);
}

// 在玩家东/西/北随机方位瞬移出现，并播放空间出现音
void AMonster::SpawnAtRandomDirection(APlayerCamera* Player)
{
	if (!Player) return;

	FVector PlayerLocation = Player->GetActorLocation();
	// 北向入口门打开前仅东/西；打开后才允许 North
	TArray<FString> Directions = { TEXT("East"), TEXT("West") };
	if (bAllowNorthSpawn)
	{
		Directions.Add(TEXT("North"));
	}
	CurrentDirection = Directions[FMath::RandRange(0, Directions.Num() - 1)];
	bIsActive = true;
	FVector SpawnLocation = PlayerLocation;
	if (CurrentDirection == "East") SpawnLocation += FVector(0.f, SpawnDistance, 100.f);
	else if (CurrentDirection == "West") SpawnLocation += FVector(0.f, -SpawnDistance, 100.f);
	else if (CurrentDirection == "North") SpawnLocation += FVector(SpawnDistance, 0.f, 100.f);
	UE_LOG(LogTemp, Display, TEXT("怪物生成在：%s"), *CurrentDirection);

	SetActorLocation(SpawnLocation);
	if (MonsterAppearSound && AppearAudioComponent)
	{
		AppearAudioComponent->SetSound(MonsterAppearSound);
		// 世界位置随 Root，与生成方位一致
		AppearAudioComponent->Play();
	}
}

// 定时逼近；超过 MaxAdvanceCount 则停生成并触发 JumpScare
void AMonster::AdvanceTowardsPlayer()
{
	AdvanceCount++;

	if (AdvanceCount > MaxAdvanceCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("Game Over! Monster reached the player."));
		GetWorldTimerManager().ClearTimer(AdvanceTimerHandle);
		bSpawnable = false;
		bTimerStarted = false;

		if (UWorld* World = GetWorld())
		{
			if (UCrankItGameplaySubsystem* Gameplay = World->GetSubsystem<UCrankItGameplaySubsystem>())
			{
				Gameplay->TriggerGameOverJumpScare(this);
			}
		}
		return;
	}

	if (!PlayerActor)
	{
		return;
	}

	// 沿玩家方向瞬移一步
	FVector Direction = (PlayerActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	SetActorLocation(GetActorLocation() + Direction * AdvanceStep);

	UE_LOG(LogTemp, Display, TEXT("Monster advanced. Count: %d"), AdvanceCount);
}

// 停逼近逻辑、对齐跳杀占位 Transform，并强制显示网格体（bSpawnable=false 后 Tick 不再改可见性）
void AMonster::BeginGameOverJumpScare(const FTransform& Pose)
{
	GetWorldTimerManager().ClearTimer(AdvanceTimerHandle);
	bSpawnable = false;
	bIsActive = false;
	bTimerStarted = false;

	SetActorTransform(Pose);
	if (MonsterBase)
	{
		MonsterBase->SetVisibility(true);
	}
}

// 灯光驱赶：隐藏逻辑上失活，延迟后重新 SpawnAtRandomDirection
void AMonster::Repel()
{
	if (!bIsActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("No active monster to repel."));
		return;
	}
	UE_LOG(LogTemp, Display, TEXT("Monster repelled by light!"));

	bIsActive = false;
	GetWorldTimerManager().ClearTimer(AdvanceTimerHandle);
	AdvanceCount = 0;

	const float DelayMin = FMath::Min(RespawnDelayMin, RespawnDelayMax);
	const float DelayMax = FMath::Max(RespawnDelayMin, RespawnDelayMax);
	const float Delay = FMath::RandRange(DelayMin, DelayMax);
	UE_LOG(LogTemp, Display, TEXT("Monster respawn in %f"), Delay);

	FTimerHandle RespawnHandle;
	GetWorldTimerManager().SetTimer(RespawnHandle, [this]()
	{
		SpawnAtRandomDirection(PlayerActor);
		GetWorldTimerManager().SetTimer(AdvanceTimerHandle, this, &AMonster::AdvanceTowardsPlayer, AdvanceInterval, true);
	}, Delay, false);
}

// bSpawnable 后首次生成，并按 bIsActive 同步网格可见性
void AMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bSpawnable)
	{
		if (!bIsSpawned)
		{
			bIsSpawned = true;
			SpawnAtRandomDirection(PlayerActor);
		}
		if (!bTimerStarted)
		{
			GetWorldTimerManager().SetTimer(AdvanceTimerHandle, this, &AMonster::AdvanceTowardsPlayer, AdvanceInterval, true);
			bTimerStarted = true;
		}
		if (!bIsActive && MonsterBase->IsVisible())
		{
			MonsterBase->SetVisibility(false);
		}
		else if (bIsActive && !MonsterBase->IsVisible())
		{
			MonsterBase->SetVisibility(true);
		}
	}
}

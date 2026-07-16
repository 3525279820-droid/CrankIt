// Fill out your copyright notice in the Description page of Project Settings.


#include "MineConsole.h"

#include "CrankItActorRegistry.h"
#include "CrankItAudioService.h"
#include "CrankItGameplaySubsystem.h"
#include "CrankItNarrativeIds.h"
#include "CrankItNarrativeSubsystem.h"
#include "PlayerCamera.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"

// Sets default values
AMineConsole::AMineConsole()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root Comp"));
	SetRootComponent(RootComp);
	

	ChargeHandle = CreateDefaultSubobject<UStaticMeshComponent>("ChargeHandle");
	ChargeHandle->SetupAttachment(RootComp);

	ChargeCellsRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ChargeCellsRoot"));
	ChargeCellsRoot->SetupAttachment(RootComp);

	for (int32 i = 0; i < NumChargeCells; ++i)
	{
		const FName CellName = *FString::Printf(TEXT("ChargeCell_%d"), i);
		UStaticMeshComponent* Cell = CreateDefaultSubobject<UStaticMeshComponent>(CellName);
		Cell->SetupAttachment(ChargeCellsRoot);
		Cell->SetVisibility(true);
		Cell->SetHiddenInGame(true);
		Cell->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ChargeCells.Add(Cell);
	}

	for(int32 i = 0; i < 3; i++)
	{
		FName SlotName = *FString::Printf(TEXT("BatterySlot_%d"), i);
		UBatterySlotTrigger* Slot = CreateDefaultSubobject<UBatterySlotTrigger>(SlotName);
		Slot->SetupAttachment(RootComp);
		Slot->SetRelativeLocation(FVector(i * 30.f, 0.f, 0.f));
	
		BatterySlots.Add(Slot);
	}
}

void AMineConsole::SetChargeCellLit(UStaticMeshComponent* Cell, bool bLit)
{
	if (!Cell)
	{
		return;
	}
	Cell->SetHiddenInGame(!bLit);
}

// BeginPlay：启动充电格衰减序列，向 ActorRegistry 注册自身并取电池对齐槽位
void AMineConsole::BeginPlay()
{
	Super::BeginPlay();

	StartLightingOffSequence();

	if (UCrankItActorRegistry* Reg = GetWorld()->GetSubsystem<UCrankItActorRegistry>())
	{
		Reg->RegisterMineConsole(this);
		Reg->GetAllBatteries(Batteries);
	}

	for(int32 i = 0; i < 3; i++){
		Battery = Batteries.IsValidIndex(i) ? Batteries[i] : nullptr;
		if(Battery){
			Battery->RootComp->SetWorldLocation(BatterySlots[i]->GetComponentLocation());
			Battery->RootComp->SetWorldRotation(BatterySlots[i]->GetComponentRotation());
		}
	}
}

// Called every frame
void AMineConsole::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (ShouldRotate)
	{
		AngularVelocityYaw = SpinVelocity;
	}

	if (FMath::Abs(AngularVelocityYaw) > KINDA_SMALL_NUMBER)
	{
		ChargeHandle->AddLocalRotation(FRotator(0, AngularVelocityYaw * DeltaTime, 0));
		AngularVelocityYaw = FMath::FInterpTo(AngularVelocityYaw, 0.f, DeltaTime, 5.0f);
	}
}

void AMineConsole::SetShouldRotate(bool CanRotate)
{
	if (ShouldRotate == CanRotate)
	{
		return;
	}

	ShouldRotate = CanRotate;

	if (UCrankItAudioService* Audio = UCrankItAudioService::Get(this))
	{
		if (CanRotate)
		{
			if (ChargeHandle && !Audio->IsPlaying(CrankLoopSoundHandle))
			{
				CrankLoopSoundHandle = Audio->PlayAttached3D(
					CrankLoopSound,
					ChargeHandle,
					NAME_None,
					1.f,
					true);
			}
		}
		else
		{
			Audio->Stop(CrankLoopSoundHandle);
		}
	}
}

void AMineConsole::StartLightingOffSequence()
{
	GetWorldTimerManager().SetTimer(
		ChargeLightTimer,
		this,
		&AMineConsole::LightOff,
		0.5,
		true
	);
}

void AMineConsole::LightNext()
{
	if (ChargeCells.Num() == 0)
	{
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("%d"), CurrentLightIndex)

	if (CurrentLightIndex < ChargeCells.Num())
	{
		SetChargeCellLit(ChargeCells[CurrentLightIndex], true);
		CurrentLightIndex++;
	}

	// 全部点亮后充一格电并重置
	if (CurrentLightIndex >= ChargeCells.Num())
	{
		AllChargeCellsOff();
		CheckNeedCharge();
	}
}

void AMineConsole::LightOff()
{
	if (CurrentLightIndex > 0 && !ShouldRotate)
	{
		CurrentLightIndex--;
		if (ChargeCells.IsValidIndex(CurrentLightIndex))
		{
			SetChargeCellLit(ChargeCells[CurrentLightIndex], false);
		}
	}
}

void AMineConsole::AllChargeCellsOff()
{
	for (UStaticMeshComponent* Cell : ChargeCells)
	{
		SetChargeCellLit(Cell, false);
	}
	CurrentLightIndex = 0;
}

// 电池充到指定格数时播放对应教程字幕
void AMineConsole::TryShowChargeTutorialSubtitle(int32 NewChargeLevel)
{
	if (bSkipedTutorial || bBatteryFirstCharged)
	{
		return;
	}

	if (NewChargeLevel <= ChargeTutorialLineShownUpTo)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FName TrackId = NAME_None;
	switch (NewChargeLevel)
	{
	case 1:
		TrackId = CrankItNarrative::Subtitle::ChargeTutorial_Level1;
		break;
	case 2:
		TrackId = CrankItNarrative::Subtitle::ChargeTutorial_Level2;
		break;
	case 3:
		TrackId = CrankItNarrative::Subtitle::ChargeTutorial_Level3;
		break;
	default:
		return;
	}

	// 按充电格数播放对应教程字幕
	if (UCrankItNarrativeSubsystem* Narrative = World->GetSubsystem<UCrankItNarrativeSubsystem>())
	{
		Narrative->PlaySubtitleTrack(TrackId);
	}

	ChargeTutorialLineShownUpTo = NewChargeLevel;
	if (NewChargeLevel >= 3)
	{
		bBatteryFirstCharged = true;
	}
}

// 写入 bSkipedTutorial，供充电/EMP 教程字幕 gate 读取
void AMineConsole::SetTutorialSkipped(bool bSkipped)
{
	bSkipedTutorial = bSkipped;
}

// 满电 EMP 触发前的前置条件：West 朝向且教程未跳过、EMP 轨未播过
bool AMineConsole::ShouldShowEMPTutorial(int32 PlayerDirectionIndex) const
{
	return PlayerDirectionIndex == EMPTutorialDirectionIndex
		&& !bSkipedTutorial
		&& !bEMPFirstTriggered;
}

// 首次触发 EMP 时播放 Gordon 教程字幕，结束后经 GameplaySubsystem 通知 IntroFlow 解锁关卡
void AMineConsole::TryShowLightTutorialSubtitle()
{
	if (bSkipedTutorial || bEMPFirstTriggered)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (PC)
	{
		APlayerCamera::DisableAllInput(PC);
	}

	if (UCrankItNarrativeSubsystem* Narrative = World->GetSubsystem<UCrankItNarrativeSubsystem>())
	{
		// 字幕播完后恢复输入、抬升探测器挂点，并经 GameplaySubsystem 通知 IntroFlow 解锁关卡
		Narrative->PlaySubtitleTrack(CrankItNarrative::Subtitle::EMP_Tutorial, [World]()
		{
			if (APlayerController* CallbackPC = World->GetFirstPlayerController())
			{
				APlayerCamera::EnableAllInput(CallbackPC);
				if (APlayerCamera* Cam = Cast<APlayerCamera>(CallbackPC->GetPawn()))
				{
					Cam->StartSoundDetectorHoldLift();
				}
				if (UCrankItGameplaySubsystem* Gameplay = World->GetSubsystem<UCrankItGameplaySubsystem>())
				{
					Gameplay->NotifyPostEMPTutorialFinished();
				}
			}
		});
	}

	bEMPFirstTriggered = true;
}

void AMineConsole::ChargeBattery()
{
	if (Battery)
	{
		if(Battery->GetChargeProgress() < 3)
		{
			const int32 NewLevel = Battery->GetChargeProgress() + 1;
			Battery->SetChargeProgress(NewLevel);
			TryShowChargeTutorialSubtitle(NewLevel);
			UE_LOG(LogTemp, Display, TEXT("Battery Charging...."))
			UE_LOG(LogTemp, Display, TEXT("Battery Level: %d"), NewLevel)
		}
	}
}

// 灯灭后检查前三块电池是否需要充电（经 ActorRegistry 刷新列表）
void AMineConsole::CheckNeedCharge()
{
	if (UCrankItActorRegistry* Reg = GetWorld()->GetSubsystem<UCrankItActorRegistry>())
	{
		Reg->GetAllBatteries(Batteries);
	}

	for(int32 i = 0; i < 3; i++)
	{
		Battery = Batteries.IsValidIndex(i) ? Batteries[i] : nullptr;
		if(Battery && !Battery->IsFullyCharged() && Battery->IsChargingEnabled()){
			ChargeBattery();
			break;
		}
	}
}

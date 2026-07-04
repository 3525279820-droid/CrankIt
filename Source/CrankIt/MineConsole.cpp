// Fill out your copyright notice in the Description page of Project Settings.


#include "MineConsole.h"

#include "CrankItActorRegistry.h"
#include "CrankItGameplaySubsystem.h"
#include "CrankItNarrativeIds.h"
#include "CrankItNarrativeSubsystem.h"
#include "PlayerCamera.h"
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

	for(int32 i = 0; i < 10; i++)
	{
		FName LightName = *FString::Printf(TEXT("ChargeLight_%d"), i);
		UPointLightComponent* PointLight = CreateDefaultSubobject<UPointLightComponent>(LightName);
		PointLight->SetupAttachment(RootComp);

		PointLight->SetRelativeLocation(FVector(i * 30.f, 0.f, 0.f));
		
		PointLight->SetVisibility(false);
		PointLight->SetIntensity(5000.f);
		PointLight->SetLightFColor(FColor(255, 0, 0, 255));

		ChargeLights.Add(PointLight);
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

// BeginPlay：启动充电灯序列，经 ActorRegistry 取电池并对齐到槽位
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
	ShouldRotate = CanRotate;
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
	UE_LOG(LogTemp, Display, TEXT("%d"), CurrentLightIndex)

	if(CurrentLightIndex < ChargeLights.Num() - 1)
	{
		ChargeLights[CurrentLightIndex]->SetVisibility(true);
		CurrentLightIndex++;
	}else
	{
		AllLightsOff();
		CheckNeedCharge();
	}
}

void AMineConsole::LightOff()
{
	if(CurrentLightIndex >= 0 and !ShouldRotate)
	{

		ChargeLights[CurrentLightIndex]->SetVisibility(false);
		if(CurrentLightIndex) CurrentLightIndex--;
	}

}

void AMineConsole::AllLightsOff()
{
	for(int32 i = 0; i < 10; i++)
	{
		ChargeLights[i]->SetVisibility(false);
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

void AMineConsole::SetTutorialSkipped(bool bSkipped)
{
	bSkipedTutorial = bSkipped;
}

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
		// 字幕播完后恢复输入并准备关卡（怪物、电脑屏幕等）
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
		if(Battery->ChargeProgress < 3)
		{
			Battery->ChargeProgress += 1;
			TryShowChargeTutorialSubtitle(Battery->ChargeProgress);
			UE_LOG(LogTemp, Display, TEXT("Battery Charging...."))
			UE_LOG(LogTemp, Display, TEXT("Battery Level: %d"), Battery->ChargeProgress)
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
		if(Battery && Battery->ChargeProgress < 3 && Battery->canCharge){
			ChargeBattery();
			break;
		}
	}
}




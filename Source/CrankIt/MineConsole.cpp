// Fill out your copyright notice in the Description page of Project Settings.


#include "MineConsole.h"

#include "InitLevel.h"
#include "Kismet/GameplayStatics.h"
#include "SubtitleSubsystem.h"
#include "PlayerCamera.h"
#include "GameFramework/PlayerController.h"
#include "CrankItNarrativeSubsystem.h"
#include "CrankItNarrativeIds.h"

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

// Called when the game starts or when spawned
void AMineConsole::BeginPlay()
{
	Super::BeginPlay();

	StartLightingOffSequence();

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABattery::StaticClass(), Batteries);

	for(int32 i = 0; i < 3; i++){
		Battery = Cast<ABattery>(Batteries[i]);
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

	/* LEGACY ChargeTutorial
	TArray<FCrankItSubtitleLine> Lines;
	switch (NewChargeLevel)
	{
	case 1:
		Lines = {
			{0.f, 2.5f, TEXT("That's it! Crank it harder!")},
		};
		break;
	case 2:
		Lines = {
			{0.f, 2.5f, TEXT("I can tell you've done this before!")},
		};
		break;
	case 3:
		Lines = {
			{0.f, 2.5f, TEXT("I bet the boys upstairs love you!")},
		};
		break;
	default:
		return;
	}
	SubtitleSys->PlaySubtitleTrack(Lines);
	*/

	ChargeTutorialLineShownUpTo = NewChargeLevel;
	if (NewChargeLevel >= 3)
	{
		bBatteryFirstCharged = true;
	}
}

// 首次触发 EMP 时播放 Gordon 教程字幕，结束后解锁关卡元素
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
					if (AInitLevel* InitLevel = World->GetAuthGameMode<AInitLevel>())
					{
						InitLevel->PrepareLevel();
					}
				}
			}
		});
	}

	/* LEGACY EMP_Tutorial
	const TArray<FCrankItSubtitleLine> Lines = {
		{0.f, 2.5f, TEXT("Argh, you stupid fucking idiot!")},
		{2.5f, 3.5f, TEXT("You almost blinded me!")},
		{3.5f, 4.f, TEXT("Just kidding.")},
		{4.f, 5.f, TEXT("I'm Gordon.")},
		{5.f, 6.f, TEXT("The light doesn't bother me.")},
		{6.f, 7.f, TEXT("I'm sorta just built different.")},
		{7.f, 8.f, TEXT("Oh, by the way,")},
		{8.f, 9.f, TEXT("You can use your decibel meter to"
			"check for sounds in the tunnels.")},
		{9.f, 10.f, TEXT("It picks up even the smallest of movements!")},
		{10.f, 10.5f, TEXT("Hmm...")},
		{10.5f, 11.5f, TEXT("That's Strange!")},
		{11.5f, 13.5f, TEXT("I'm getting some incredibly large"
			"seismic activity down there!")},
		{13.5f, 14.5f, TEXT("Maybe it's your mom?")},
		{14.5f, 15.f, TEXT("Ha!")},
		{15.f, 16.5f, TEXT("Mm, ima go check it out.")},
		{16.5f, 18.5f, TEXT("See you later cranker!")},
		{18.5f, 20.5f, TEXT("[unintelligible]")},
	};

	APlayerController* PC = World->GetFirstPlayerController();
	if (PC)
	{
		APlayerCamera::DisableAllInput(PC);
	}

	SubtitleSys->PlaySubtitleTrack(Lines, [World]()
	{
		if (APlayerController* CallbackPC = World->GetFirstPlayerController())
		{
			APlayerCamera::EnableAllInput(CallbackPC);
			if (APlayerCamera* Cam = Cast<APlayerCamera>(CallbackPC->GetPawn()))
			{
				Cam->StartSoundDetectorHoldLift();
				if (AInitLevel* InitLevel = World->GetAuthGameMode<AInitLevel>())
				{
					InitLevel->PrepareLevel();
				}
			}
		}

	});
	*/

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

void AMineConsole::CheckNeedCharge()
{
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABattery::StaticClass(), Batteries);

	for(int32 i = 0; i < 3; i++)
	{
		Battery = Cast<ABattery>(Batteries[i]);
		if(Battery && Battery->ChargeProgress < 3 && Battery->canCharge){
			ChargeBattery();
			break;
		}
	}
}




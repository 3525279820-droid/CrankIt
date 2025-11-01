// Fill out your copyright notice in the Description page of Project Settings.


#include "MineConsole.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
AMineConsole::AMineConsole()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root Comp"));
	SetRootComponent(RootComp);
	
	ConsoleBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ConsoleBase"));
	ConsoleBase->SetupAttachment(RootComp);

	ChargeHandle = CreateDefaultSubobject<UStaticMeshComponent>("ChargeHandle");
	ChargeHandle->SetupAttachment(ConsoleBase);

	for(int32 i = 0; i < 10; i++)
	{
		FName LightName = *FString::Printf(TEXT("ChargeLight_%d"), i);
		UPointLightComponent* PointLight = CreateDefaultSubobject<UPointLightComponent>(LightName);
		PointLight->SetupAttachment(ConsoleBase);

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
		Slot->SetupAttachment(ConsoleBase);
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
		AngularVelocityYaw = 800.f;
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

void AMineConsole::ChargeBattery()
{
	if (Battery)
	{
		if(Battery->ChargeProgress < 3)
		{
			Battery->ChargeProgress += 1;
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
		if(Battery && Battery->ChargeProgress < 3){
			ChargeBattery();
			break;
		}
	}
}




// Fill out your copyright notice in the Description page of Project Settings.


#include "TriggerComp.h"

void UTriggerComp::BeginPlay()
{
	Super::BeginPlay();
	OnComponentEndOverlap.AddDynamic(this, &UTriggerComp::OnOverlapEnd);

}

UTriggerComp::UTriggerComp()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTriggerComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UTriggerComp::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	bool HasTag = OverlappedComp ? OtherComp->ComponentHasTag("ChargeHandleBase") : false;
	if (HasTag)
	{
		if (AMineConsole* Console = Cast<AMineConsole>(GetOwner()))
		{
			Console->LightNext();
		}
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("tag no found.."))
		UE_LOG(LogTemp, Display, TEXT("%s"), *OtherComp->GetName())
	}
}

void UTriggerComp::SetProgress()
{
}

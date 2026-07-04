#include "TerminalBatteryHoldController.h"

#include "TerminalWidget.h"
#include "TerminalDisplayController.h"
#include "Battery.h"
#include "BatteryPowerChecker.h"
#include "CrankItNarrativeIds.h"

void UTerminalBatteryHoldController::Initialize(UTerminalWidget* InHost, UTerminalDisplayController* InDisplay)
{
	Host = InHost;
	Display = InDisplay;
}

void UTerminalBatteryHoldController::Configure(
	int32 InRequiredCount,
	int32 InMinChargeLevel,
	float InHoldDurationSeconds,
	float InTickInterval,
	float InDrainInterval,
	int32 InDrainAmount)
{
	RequiredChargedBatteryCount = InRequiredCount;
	MinBatteryChargeLevel = InMinChargeLevel;
	BatteryHoldDurationSeconds = InHoldDurationSeconds;
	BatteryHoldTickInterval = InTickInterval;
	BatteryDrainInterval = InDrainInterval;
	BatteryDrainAmount = InDrainAmount;
}

void UTerminalBatteryHoldController::SetGatedCommands(const TSet<FString>& Commands)
{
	BatteryGatedCommands = Commands;
}

void UTerminalBatteryHoldController::Shutdown()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BatteryHoldTimerHandle);
	}
	bBatteryHoldActive = false;
}

void UTerminalBatteryHoldController::TryStart(const FString& Command)
{
	if (!Host || !Display || !BatteryGatedCommands.Contains(Command))
	{
		return;
	}

	if (bBatteryHoldActive)
	{
		Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::Battery_TransferAlreadyActive);
		Display->StartDisplayingLines();
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (!FBatteryPowerChecker::HasEnoughPower(World, RequiredChargedBatteryCount, MinBatteryChargeLevel))
	{
		Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::Battery_LowPowerError);
		Display->StartDisplayingLines();
		return;
	}

	const float Now = World->GetTimeSeconds();
	bBatteryHoldActive = true;
	BatteryHoldEndTime = Now + BatteryHoldDurationSeconds;
	BatteryHoldLastDrainTime = Now;
	BatteryHoldLastProgressTime = Now;
	Display->ClearInputLine();
	Display->UpdateDisplay();

	Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::Battery_TransferStarted);
	Display->AddPendingLine(FString::Printf(
		TEXT("Transfer remaining: %d seconds"),
		FMath::CeilToInt(BatteryHoldDurationSeconds)));
	Display->AddPendingLine(TEXT(""));
	Display->StartDisplayingLines();

	World->GetTimerManager().SetTimer(
		BatteryHoldTimerHandle,
		this,
		&UTerminalBatteryHoldController::OnBatteryHoldTick,
		BatteryHoldTickInterval,
		true
	);
}

void UTerminalBatteryHoldController::OnBatteryHoldTick()
{
	if (!Display || !Host || !bBatteryHoldActive)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		EndBatteryHold(false);
		return;
	}

	const float Now = World->GetTimeSeconds();

	if (!FBatteryPowerChecker::HasEnoughPower(World, RequiredChargedBatteryCount, MinBatteryChargeLevel))
	{
		EndBatteryHold(false);
		return;
	}

	if (BatteryDrainInterval > KINDA_SMALL_NUMBER && Now - BatteryHoldLastDrainTime >= BatteryDrainInterval)
	{
		BatteryHoldLastDrainTime = Now;
		TArray<ABattery*> Batteries;
		FBatteryPowerChecker::GetBatteriesInTerminalSlots(World, MinBatteryChargeLevel, Batteries);
		for (ABattery* Battery : Batteries)
		{
			if (Battery)
			{
				Battery->SetChargeProgress(FMath::Max(0, Battery->GetChargeProgress() - BatteryDrainAmount));
			}
		}
		if (!FBatteryPowerChecker::HasEnoughPower(World, RequiredChargedBatteryCount, MinBatteryChargeLevel))
		{
			EndBatteryHold(false);
			return;
		}
	}

	if (Now - BatteryHoldLastProgressTime >= 10.f)
	{
		BatteryHoldLastProgressTime = Now;
		Display->AddPendingLine(FString::Printf(
			TEXT("Transfer remaining: %d seconds"),
			FMath::CeilToInt(FMath::Max(0.f, BatteryHoldEndTime - Now))));
		Display->StartDisplayingLines();
	}

	if (Now >= BatteryHoldEndTime)
	{
		EndBatteryHold(true);
	}
}

void UTerminalBatteryHoldController::EndBatteryHold(bool bSuccess)
{
	if (!Display || !Host || !bBatteryHoldActive)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BatteryHoldTimerHandle);
	}

	bBatteryHoldActive = false;
	BatteryHoldEndTime = 0.f;

	if (bSuccess)
	{
		Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::Battery_TransferComplete);
	}
	else
	{
		Host->AppendTerminalOutputBlock(CrankItNarrative::Terminal::Battery_TransferInterrupted);
	}
	Display->StartDisplayingLines();
}

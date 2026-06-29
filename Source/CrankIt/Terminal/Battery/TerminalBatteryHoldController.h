#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TerminalBatteryHoldController.generated.h"

class UTerminalWidget;
class UTerminalDisplayController;

/** Terminal/Battery — 终端槽位供电 Hold：定时扣电、进度提示、阻塞输入 */
UCLASS()
class CRANKIT_API UTerminalBatteryHoldController : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UTerminalWidget* InHost, UTerminalDisplayController* InDisplay);

	bool IsActive() const { return bBatteryHoldActive; }

	void Configure(
		int32 InRequiredCount,
		int32 InMinChargeLevel,
		float InHoldDurationSeconds,
		float InTickInterval,
		float InDrainInterval,
		int32 InDrainAmount);

	void SetGatedCommands(const TSet<FString>& Commands);

	void TryStart(const FString& Command);

	void Shutdown();

private:
	UFUNCTION()
	void OnBatteryHoldTick();

	void EndBatteryHold(bool bSuccess);

	UPROPERTY()
	TObjectPtr<UTerminalWidget> Host;

	UPROPERTY()
	TObjectPtr<UTerminalDisplayController> Display;

	TSet<FString> BatteryGatedCommands;

	bool bBatteryHoldActive = false;
	FTimerHandle BatteryHoldTimerHandle;
	float BatteryHoldEndTime = 0.f;
	float BatteryHoldLastDrainTime = 0.f;
	float BatteryHoldLastProgressTime = 0.f;

	int32 RequiredChargedBatteryCount = 1;
	int32 MinBatteryChargeLevel = 1;
	float BatteryHoldDurationSeconds = 60.f;
	float BatteryHoldTickInterval = 0.5f;
	float BatteryDrainInterval = 20.f;
	int32 BatteryDrainAmount = 1;
};

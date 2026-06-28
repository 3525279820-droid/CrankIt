#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Monster.h"
#include "FlashTopLight.h"
#include "Chaos/Framework/HashMappedArray.h"
#include "TerminalDisplayController.h"
#include "TerminalWidget.generated.h"

class UEditableTextBox;
class UTextBlock;
class UClassificationGameWidget;
class UCalibrationWidget;

UENUM(BlueprintType)
enum class ETerminalInputMode : uint8
{
	Terminal UMETA(DisplayName = "Terminal"),
	ClassificationGame UMETA(DisplayName = "ClassificationGame"),
	CalibrationGame UMETA(DisplayName = "CalibrationGame")
};

UCLASS()
class CRANKIT_API UTerminalWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void GenerateTarget();
	void ResetRound();
	void AddToGuessHistory(const FString& Guess);
	virtual void NativeConstruct() override;
	void StartDisplayingLinesProcedure(float Interval, TFunction<void()> OnProcedureComplete = TFunction<void()>());
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TerminalText;

	UPROPERTY(EditAnywhere)
	float Interval = 0.2f;

	UFUNCTION(BlueprintCallable, Category="Terminal")
	void AddNewLine(const FString& Line);

	UFUNCTION(BlueprintCallable, Category="Terminal")
	void AddNewLines(const TArray<FString>& Lines);

protected:
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	void CommitGuessNum();

private:
	FString PendingCommandKey;

	bool BinaryGuessActivated = false;
	FString NewTarget;
	int32 MaxGuesses = 7;
	TArray<FString> GuessHistory;
	int32 CurrentGuessCount = 0;

	int32 BinaryLength = 6;

	void CommitInput();

	TMap<FString, TArray<FString>> CommandTextMap;
	TMap<FString, TFunction<void()>> CommandActionMap;
	TSet<FString> BatteryGatedCommands;

	UPROPERTY(EditAnywhere, Category="Terminal")
	TArray<FString> CommandSequence;

	int32 NextCommandIndex = 0;
	bool bErrorShownForCurrentCommand = false;
	FString CurrentStageErrorText;
	FString CurrentStageWrongInputText;

	void ClearPreviousWrongAttemptFromScreen();
	void ShowStageCommandError(const FString& SubmittedInput);

	// --- 叙事 Data Asset 接入 ---
	void LoadTerminalCommandDataFromAsset();   // 加载命令顺序与 CommandTextMap
	void SetupCommandActions();                // 注册命令副作用（不含文案）
	void AppendTerminalOutputBlock(FName BlockId); // 从 Data Asset 追加终端输出块

	UPROPERTY()
	TObjectPtr<UTerminalDisplayController> Display;

public:
	UFUNCTION(BlueprintCallable, Category="Terminal|MiniGame")
	void EnterClassificationGame();

	UFUNCTION(BlueprintCallable, Category="Terminal|MiniGame")
	void ExitClassificationGame();

	UFUNCTION()
	void HandleClassificationGameFinished(bool bAllCorrect);

	UFUNCTION(BlueprintCallable, Category="Terminal|MiniGame")
	void EnterCalibrationGame();

	UFUNCTION(BlueprintCallable, Category="Terminal|MiniGame")
	void ExitCalibrationGame();

	UFUNCTION()
	void HandleCalibrationGameFinished(bool bWon);

	UPROPERTY(BlueprintReadOnly, Category="Terminal|MiniGame")
	ETerminalInputMode CurrentInputMode = ETerminalInputMode::Terminal;

private:
	bool bInClassificationGame = false;

	UPROPERTY(meta=(BindWidgetOptional))
	UClassificationGameWidget* ClassificationGameWidget = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UCalibrationWidget* CalibrationWidget = nullptr;

	void TryStartBatteryHold(const FString& Command);
	void OnBatteryHoldTick();
	void EndBatteryHold(bool bSuccess);

	bool bBatteryHoldActive = false;
	FTimerHandle BatteryHoldTimerHandle;
	float BatteryHoldEndTime = 0.f;
	float BatteryHoldLastDrainTime = 0.f;
	float BatteryHoldLastProgressTime = 0.f;

	UPROPERTY(EditAnywhere, Category = "Terminal|Battery")
	int32 RequiredChargedBatteryCount = 1;

	UPROPERTY(EditAnywhere, Category = "Terminal|Battery")
	int32 MinBatteryChargeLevel = 1;

	UPROPERTY(EditAnywhere, Category = "Terminal|Battery")
	float BatteryHoldDurationSeconds = 60.f;

	UPROPERTY(EditAnywhere, Category = "Terminal|Battery")
	float BatteryHoldTickInterval = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Terminal|Battery")
	float BatteryDrainInterval = 20.f;

	UPROPERTY(EditAnywhere, Category = "Terminal|Battery")
	int32 BatteryDrainAmount = 1;
};

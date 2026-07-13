#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TerminalSharedTypes.h"
#include "TerminalDisplayController.h"
#include "TerminalCommandRouter.h"
#include "TerminalActionDispatcher.h"
#include "BoardleGameController.h"
#include "TerminalWidget.generated.h"

class UTerminalMiniGameHost;
class UTerminalBatteryHoldController;
class UClassificationGameWidget;
class UCalibrationWidget;
class USoundBase;

/** Terminal/Host — 终端壳：键盘路由、命令提交；显示 / 路由 / 副作用 / 小游戏 / BOORDLE 分模块 */
UCLASS()
class CRANKIT_API UTerminalWidget : public UUserWidget
{
	GENERATED_BODY()

	friend class UTerminalActionDispatcher;
	friend class UTerminalMiniGameHost;
	friend class UTerminalBatteryHoldController;

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void StartDisplayingLinesProcedure(float Interval, TFunction<void()> OnProcedureComplete = TFunction<void()>());

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TerminalText;

	UPROPERTY(EditAnywhere)
	float Interval = 0.2f;

	UFUNCTION(BlueprintCallable, Category = "Terminal")
	void AddNewLine(const FString& Line);

	UFUNCTION(BlueprintCallable, Category = "Terminal")
	void AddNewLines(const TArray<FString>& Lines);

	/** ActionDispatcher：BOORDLE 命令入口 */
	void StartBoardleGame();

	void AppendTerminalOutputBlock(FName BlockId);

	/** 供 ActionDispatcher 或命令白名单调用（BatteryGatedCommands 未配置时无效果） */
	void TryStartBatteryHold(const FString& Command);

	UFUNCTION(BlueprintCallable, Category = "Terminal|MiniGame")
	void EnterClassificationGame();

	UFUNCTION(BlueprintCallable, Category = "Terminal|MiniGame")
	void ExitClassificationGame();

	UFUNCTION(BlueprintCallable, Category = "Terminal|MiniGame")
	void EnterCalibrationGame();

	UFUNCTION(BlueprintCallable, Category = "Terminal|MiniGame")
	void ExitCalibrationGame();

	UPROPERTY(BlueprintReadOnly, Category = "Terminal|MiniGame")
	ETerminalInputMode CurrentInputMode = ETerminalInputMode::Terminal;

protected:
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	void CommitInput();
	void CommitBoardleGuess();
	void ApplyBoardleOutcomes(const TArray<FBoardleGuessOutcome>& Outcomes);
	void LoadCommandRouterFromAsset();
	void ClearPreviousWrongAttemptFromScreen();
	void ShowStageCommandError(const FString& SubmittedInput);
	void RequestExitComputerView();

	FBoardleGameController BoardleGame;

	FTerminalCommandRouter CommandRouter;

	UPROPERTY()
	TObjectPtr<UTerminalDisplayController> Display;

	UPROPERTY()
	TObjectPtr<UTerminalActionDispatcher> ActionDispatcher;

	UPROPERTY()
	TObjectPtr<UTerminalMiniGameHost> MiniGameHost;

	UPROPERTY()
	TObjectPtr<UTerminalBatteryHoldController> BatteryHold;

	bool bErrorShownForCurrentCommand = false;
	FString CurrentStageErrorText;
	FString CurrentStageWrongInputText;

	UPROPERTY(meta = (BindWidgetOptional))
	UClassificationGameWidget* ClassificationGameWidget = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UCalibrationWidget* CalibrationWidget = nullptr;

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

	UPROPERTY(EditAnywhere, Category = "Terminal|Audio")
	TObjectPtr<USoundBase> KeyInputSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "Terminal|Audio")
	TObjectPtr<USoundBase> BackspaceSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "Terminal|Audio")
	TObjectPtr<USoundBase> SpaceSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "Terminal|Audio")
	TObjectPtr<USoundBase> UnknownCommandSound = nullptr;

	void PlayTerminalSound2D(USoundBase* Sound);

	void ScheduleBoardleScreenClear(TFunction<void()> AfterClear);

	FTimerHandle BoardleDeferredTimerHandle;
};

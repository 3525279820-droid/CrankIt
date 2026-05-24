#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Chaos/Framework/HashMappedArray.h"
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
	void StartDisplayingLinesProcedure(float Interval);
	virtual void NativeDestruct() override;
	// 在蓝图中绑定 TextBlock
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TerminalText;

	UPROPERTY(EditAnywhere)
	float Interval = 0.2;
	
	UFUNCTION(BlueprintCallable, Category="Terminal")
	void AddNewLine(const FString& Line);

	UFUNCTION(BlueprintCallable, Category="Terminal")
	void AddNewLines(const TArray<FString>& Lines);

protected:
	void StartDisplayingLines();
	void DisplayNextLine();

	UPROPERTY()
	TArray<FString> PendingLines; // 将待显示的行压入栈中，等待处理

	FTimerHandle DisplayTimerHandle;
	
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	void CommitGuessNum();

private:
	FString CurrentText;
	FString CurrentInputLine;
	
	bool BinaryGuessActivated = false;
	FString NewTarget;
	int32 MaxGuesses = 7;                   // 每轮最大猜测次数
	TArray<FString> GuessHistory;           // 保存每次猜测的字符串（"010101"）
	int32 CurrentGuessCount = 0;            // 已猜次数

	// UI / 行为
	int32 BinaryLength = 6;      
	
	void UpdateDisplay();
	void CommitInput();
	void ClearTerminal();
	// 文本命令表：命令 -> 多行文本
	TMap<FString, TArray<FString>> CommandTextMap;

	// 行为命令表：命令 -> 可执行逻辑（lambda 或绑定函数）
	TMap<FString, TFunction<void()>> CommandActionMap;

	/** 需要检查 TerminalSlot 电池的命令（在 CommandActionMap 中调用 TryStartBatteryHold） */
	TSet<FString> BatteryGatedCommands;

	// 顺序命令队列
	UPROPERTY(EditAnywhere, Category="Terminal")
	TArray<FString> CommandSequence;

	// 当前需要匹配的索引（指向 CommandSequence 的下一个命令）
	int32 NextCommandIndex = 0;

	// ========== 分类小游戏相关 ==========
public:
	// 由终端命令触发，进入分类小游戏模式
	UFUNCTION(BlueprintCallable, Category="Terminal|MiniGame")
	void EnterClassificationGame();

	// 小游戏结束时调用，恢复终端输入模式
	UFUNCTION(BlueprintCallable, Category="Terminal|MiniGame")
	void ExitClassificationGame();

	// 动态委托回调签名（给 OnGameFinished 绑定用）
	UFUNCTION()
	void HandleClassificationGameFinished(bool bAllCorrect);

	UFUNCTION(BlueprintCallable, Category="Terminal|MiniGame")
	void EnterCalibrationGame();

	UFUNCTION(BlueprintCallable, Category="Terminal|MiniGame")
	void ExitCalibrationGame();

	UFUNCTION()
	void HandleCalibrationGameFinished(bool bWon);

	// 当前输入模式（终端 / 分类小游戏）
	UPROPERTY(BlueprintReadOnly, Category="Terminal|MiniGame")
	ETerminalInputMode CurrentInputMode = ETerminalInputMode::Terminal;

private:
	// 标记当前是否处于分类小游戏中，和 CurrentInputMode 保持一致，只在 C++ 内部使用
	bool bInClassificationGame = false;
	
	// 分类小游戏 Widget（在终端 Widget 蓝图里放一个同名子控件即可自动绑定）
	UPROPERTY(meta=(BindWidgetOptional))
	UClassificationGameWidget* ClassificationGameWidget = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UCalibrationWidget* CalibrationWidget = nullptr;
	// ========== 分类小游戏相关 End ==========

	// ========== 终端槽位供电（仅 BatteryGatedCommands 白名单命令）==========
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

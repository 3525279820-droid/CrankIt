#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Chaos/Framework/HashMappedArray.h"
#include "TerminalWidget.generated.h"

class UEditableTextBox;
class UTextBlock;

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
	// 文本命令表：命令 -> 多行文本
	TMap<FString, TArray<FString>> CommandTextMap;

	// 行为命令表：命令 -> 可执行逻辑（lambda 或绑定函数）
	TMap<FString, TFunction<void()>> CommandActionMap;

	// 顺序命令队列
	UPROPERTY(EditAnywhere, Category="Terminal")
	TArray<FString> CommandSequence;

	// 当前需要匹配的索引（指向 CommandSequence 的下一个命令）
	int32 NextCommandIndex = 0;


};

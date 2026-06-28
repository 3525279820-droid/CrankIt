#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TerminalDisplayController.generated.h"

class UTextBlock;
class UTerminalWidget;

/** 终端逐行显示与 TextBlock 刷新；不含命令解析或玩法副作用 */
UCLASS()
class CRANKIT_API UTerminalDisplayController : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UTerminalWidget* InOwner, UTextBlock* InTerminalText);

	UPROPERTY(EditAnywhere, Category = "Terminal")
	float Interval = 0.2f;

	void StartDisplayingLines();
	void DisplayNextLine();
	void StartDisplayingLinesProcedure(float Delay, TFunction<void()> OnProcedureComplete = TFunction<void()>());

	void UpdateDisplay();
	void ClearTerminal();

	void AddNewLine(const FString& Line);
	void AddNewLines(const TArray<FString>& Lines);

	void AddPendingLine(const FString& Line);
	void AppendPendingLines(const TArray<FString>& Lines);
	void ClearPendingLines();
	void RemovePendingLinesIf(TFunctionRef<bool(const FString&)> Predicate);

	void ClearDisplayTimer();
	bool IsDisplayTimerActive() const;
	void SetOneShotTimer(float DelaySeconds, TFunction<void()> Callback);

	FString GetCurrentInputLine() const { return CurrentInputLine; }
	void SetCurrentInputLine(const FString& Line);
	void AppendCharToInputLine(TCHAR Char);
	void RemoveLastInputChar();
	void ClearInputLine();

	FString& GetCurrentText() { return CurrentText; }
	const FString& GetCurrentText() const { return CurrentText; }
	void AppendToCurrentText(const FString& Text);
	bool CurrentTextEndsWith(const FString& Suffix) const;
	void RemoveCurrentTextSuffix(const FString& Suffix);

private:
	UWorld* GetWorld() const;

	UPROPERTY()
	TObjectPtr<UTerminalWidget> OwnerWidget;

	UPROPERTY()
	TObjectPtr<UTextBlock> TerminalText;

	UPROPERTY()
	TArray<FString> PendingLines;

	FTimerHandle DisplayTimerHandle;

	FString CurrentText;
	FString CurrentInputLine;

	TFunction<void()> ProcedureOnComplete;
};

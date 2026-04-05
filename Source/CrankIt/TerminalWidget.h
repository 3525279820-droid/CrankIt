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
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// 在蓝图中绑定 TextBlock
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TerminalText;

	UPROPERTY(EditAnywhere)
	float AddLineDelay = 1.8f;

	// 添加新行
	UFUNCTION(BlueprintCallable, Category="Terminal")
	void AddNewLine(const FString& Line);

	// 文本命令表：命令 -> 多行文本
	TMap<FString, TArray<FString>> CommandTextMap;

	// 行为命令表：命令 -> 可执行逻辑（lambda 或绑定函数）
	TMap<FString, TFunction<void()>> CommandActionMap;

protected:
	void StartDisplayingLines(float Interval = 0.5f);
	void DisplayNextLine();

	UPROPERTY()
	TArray<FString> PendingLines; // 将待显示的行压入栈中，等待处理

	FTimerHandle DisplayTimerHandle;
	
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	
private:
	FString CurrentText;
	FString CurrentInputLine;
	void UpdateDisplay();
	void CommitInput();

};

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TerminalWidget.generated.h"

class UEditableTextBox;
class UTextBlock;

UCLASS()
class CRANKIT_API UTerminalWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 在蓝图中绑定 TextBlock
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TerminalText;

	// 添加新行
	UFUNCTION(BlueprintCallable, Category="Terminal")
	void AddNewLine(const FString& Line);

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	
private:
	FString CurrentText;
	FString CurrentInputLine;
	void UpdateDisplay();
	void CommitInput();
};

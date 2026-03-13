#include "TerminalWidget.h"

#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerCamera.h"

void UTerminalWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// CurrentText = TEXT("");
	CurrentInputLine = TEXT("");
    SetIsFocusable(true);
	// bIsFocusable = true;
	// SetKeyboardFocus();
	// FSlateApplication::Get().SetKeyboardFocus(TakeWidget());
}

FReply UTerminalWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();
 
    if (Key == EKeys::Enter)
    {
        CommitInput();
        return FReply::Handled();
    }
    else if (Key == EKeys::BackSpace)
    {
        if (!CurrentInputLine.IsEmpty())
        {
            CurrentInputLine.RemoveAt(CurrentInputLine.Len() - 1);
            UpdateDisplay();
        }
        return FReply::Handled();
 
    }
    else if (Key == EKeys::Tab)
    {
        // 终端获得键盘焦点时，Tab 不会再传递到 Pawn 的输入映射
        // 这里主动调用 PlayerCamera 的退出逻辑
        if (UWorld* World = GetWorld())
        {
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                if (APawn* Pawn = PC->GetPawn())
                {
                    if (APlayerCamera* PlayerCamera = Cast<APlayerCamera>(Pawn))
                    {
                        PlayerCamera->ExitScreenInput(FInputActionValue());
                    }
                }
            }
        }

        return FReply::Handled();
    }
	else
    {
        // 捕获字符输入
    	uint32 CharCode = InKeyEvent.GetCharacter();
    	if (CharCode != 0) // 0 表示没有有效字符
    	{
    		TCHAR Char = (TCHAR)CharCode;
    		CurrentInputLine.AppendChar(Char);
    		UpdateDisplay();
    	}
 
    }

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UTerminalWidget::UpdateDisplay()
{
    if (TerminalText)
    {
        TerminalText->SetText(FText::FromString(CurrentText + CurrentInputLine));
    }
}

void UTerminalWidget::CommitInput()
{
    CurrentText += CurrentInputLine + TEXT("\n");
    // 在这里解析命令
    if (CurrentInputLine == TEXT("JUMP"))
    {
        // 触发游戏逻辑
    }

    CurrentInputLine.Empty();
    UpdateDisplay();
}

void UTerminalWidget::AddNewLine(const FString& Line)
{
	CurrentText += Line + TEXT("\n");
	UpdateDisplay();
}

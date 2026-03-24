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

    
    // 初始化文本命令
    CommandTextMap.Add(TEXT("JUMP"), { TEXT("Hello world!"), TEXT("compile..."), TEXT("Done!") });
    CommandTextMap.Add(TEXT("HELLO"), { TEXT("Hi!"), TEXT("Welcome.") });

    // 初始化行为命令（绑定成员函数或 lambda）
    CommandActionMap.Add(TEXT("CLEAR"), [this]()
    {
        if (TerminalText)
        {
            TerminalText->SetText(FText::GetEmpty());
        }
        CurrentText.Empty();
    });

    // 行为命令模板
    CommandActionMap.Add(TEXT("DO_SOMETHING"), [this]()
    {
        // 调用游戏逻辑、触发事件、播放音效等
        UE_LOG(LogTemp, Log, TEXT("DO_SOMETHING executed"));
    });
}

void UTerminalWidget::NativeDestruct()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(DisplayTimerHandle);
    }
    Super::NativeDestruct();
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

void UTerminalWidget::CommitInput()
{
    // 先把输入行追加到 CurrentText（如果需要）
    CurrentText += CurrentInputLine + TEXT("\n");

    // 优先查行为命令表
    if (CommandActionMap.Contains(CurrentInputLine))
    {
        // 执行行为（同步）
        CommandActionMap[CurrentInputLine]();
    }
    else if (CommandTextMap.Contains(CurrentInputLine))
    {
        // 文本命令：把对应行加入 PendingLines 并启动定时器显示
        PendingLines.Append(CommandTextMap[CurrentInputLine]);
        StartDisplayingLines(AddLineDelay);
    }
    else
    {
        // 未知命令：可显示提示或直接把输入当作普通文本
        PendingLines.Add(FString::Printf(TEXT("Unknown command: %s"), *CurrentInputLine));
        StartDisplayingLines(AddLineDelay);
    }

    CurrentInputLine.Empty();
    UpdateDisplay();
}

void UTerminalWidget::AddNewLine(const FString& Line)
{
    PendingLines.Add(Line);
    StartDisplayingLines(AddLineDelay);
}

void UTerminalWidget::StartDisplayingLines(float Interval)
{
    if (!GetWorld()) return;

    if (GetWorld()->GetTimerManager().IsTimerActive(DisplayTimerHandle))
    {
        return;
    }

    GetWorld()->GetTimerManager().SetTimer(
        DisplayTimerHandle,
        this,
        &UTerminalWidget::DisplayNextLine,
        Interval,
        true
    );
}

void UTerminalWidget::DisplayNextLine()
{
    if (PendingLines.Num() == 0)
    {
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(DisplayTimerHandle);
        }
        return;
    }

    FString Line = PendingLines[0];
    PendingLines.RemoveAt(0);

    CurrentText += Line + TEXT("\n");
    UpdateDisplay();
}

void UTerminalWidget::UpdateDisplay()
{
    if (TerminalText)
    {
        TerminalText->SetText(FText::FromString(CurrentText + CurrentInputLine));
    }
}
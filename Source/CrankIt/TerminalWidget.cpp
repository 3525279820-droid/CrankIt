#include "TerminalWidget.h"

#include <string>

#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerCamera.h"
#include "Sound/SoundWave.h"

void UTerminalWidget::GenerateTarget()
{
    
    for(int32 i = 0 ;i< BinaryLength; ++i)
    {
        bool b = FMath::RandBool();
        FString s = b ? "1" : "0";
        NewTarget += s;
    }
    
}

void UTerminalWidget::ResetRound()
{
    GuessHistory.Empty();
    CurrentGuessCount = 0;
    GenerateTarget();
}

void UTerminalWidget::AddToGuessHistory(const FString& Guess)
{
    GuessHistory.Add(Guess);
    CurrentGuessCount++;

    int32 matchCount = 0;
    for(int32 i = 0; i < BinaryLength; ++i)
    {
        if(Guess[i] == NewTarget[i])
        {
            matchCount++;
        }
    }
    PendingLines.Add(FString::Printf(TEXT("Guess %d: %s  (matching bits: %d)"), CurrentGuessCount, *Guess, matchCount));
    if (matchCount == BinaryLength)
    {
        PendingLines.Add(TEXT(">>> Correct! You found the number."));
        PendingLines.Add(FString::Printf(TEXT("Target decimal: %s"), *NewTarget));
        PendingLines.Add(TEXT("Starting new round..."));
        StartDisplayingLines();
    }
    if (CurrentGuessCount >= MaxGuesses)
    {
        PendingLines.Add(TEXT(">>> Out of guesses. Target will refresh."));
        PendingLines.Add(FString::Printf(TEXT("Target decimal was: %s"), *NewTarget));
        PendingLines.Add(TEXT("Starting new round..."));
        StartDisplayingLines();
        // 延迟重置
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimer(DisplayTimerHandle, this, &UTerminalWidget::ResetRound, 2.5f, false);
        }
    }
    StartDisplayingLines();
}

void UTerminalWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// CurrentText = TEXT("");
	CurrentInputLine = TEXT("");
    SetIsFocusable(true);

    // 初始化猜数字小游戏
    BinaryLength = 6;
    MaxGuesses = 7;
    ResetRound();

    ////////////// 初始化终端交互逻辑 ///////////////////////////////////////////////////////
    
    // 初始化命令顺序
    CommandSequence = { TEXT("BOORDLE")};
    NextCommandIndex = 0;
    
    // 初始化文本命令
    CommandTextMap.Add(TEXT("JUMP"), { TEXT("Hello world!"), TEXT("compile..."), TEXT("Done!") });
    CommandTextMap.Add(TEXT("HELLO"), { TEXT("Hi!"), TEXT("Welcome.") });
    CommandTextMap.Add(TEXT("REBOOT"), {
        TEXT("cOS [Version 12.0.19248.417] "),
        TEXT("(c) 1998 JTEC corporation. All rights reserved."),
        TEXT(""),
        TEXT("safe mode is active "),
        TEXT("An unexpected crash has occurred! "),
        TEXT(" to fix potential faults type: "),
        TEXT("SCAN AND REPAIR")
    });
    CommandTextMap.Add(TEXT("SCAN AND REPAIR"), {
        TEXT(""),
        TEXT("Scanning Files "),
        TEXT(" "),
        TEXT("Checking BIOS ...OK"),
        TEXT("Checking OS ...OK"),
        TEXT("Checking Data ...OK"),
        TEXT("SCAN AND REPAIR")
    });
    
    

    // 初始化行为命令（绑定成员函数或 lambda）
    CommandActionMap.Add(TEXT("CLEAR"), [this]()
    {
        if (TerminalText)
        {
            TerminalText->SetText(FText::GetEmpty());
        }
        CurrentText.Empty();
    });

    CommandActionMap.Add(TEXT("REBOOT"), [this]()
    {
        PendingLines.Append(CommandTextMap[CurrentInputLine]);
        StartDisplayingLinesProcedure(3.f);        
    });
    
    CommandActionMap.Add(TEXT("SCAN AND REPAIR"), [this]()
    {
        PendingLines.Append(CommandTextMap[CurrentInputLine]);
        StartDisplayingLines();
        UE_LOG(LogTemp, Display, TEXT("scanning"))
        PendingLines.Append({
            TEXT(""),
            TEXT("Scanning finished"),
            TEXT("Finished FAULTS FOUND (1): "),
            TEXT("SystemTools.bin is malformed "),
            TEXT("Suggestion use BOORDLE to identify binary fault ")
        });
        StartDisplayingLinesProcedure(1.f);        
    });
    CommandActionMap.Add(TEXT("BOORDLE"), [this]()
    {
        BinaryGuessActivated = true;
        PendingLines.Append({
            TEXT("Binary Guess Game Started"),
        });
        StartDisplayingLines();        
    });

    // 行为命令模板
    CommandActionMap.Add(TEXT("DO_SOMETHING"), [this]()
    {
        // 调用游戏逻辑、触发事件、播放音效等
        UE_LOG(LogTemp, Log, TEXT("DO_SOMETHING executed"));
    });
    //////////////////////////////////////////////////////////////////////////////////
}


void UTerminalWidget::StartDisplayingLinesProcedure(float delay)
{
    if (!GetWorld()) return;

    if (GetWorld()->GetTimerManager().IsTimerActive(DisplayTimerHandle))
    {
        return;
    }
    UE_LOG(LogTemp, Display, TEXT("rebooting"))

    GetWorld()->GetTimerManager().SetTimer(
        DisplayTimerHandle,
        this,
        &UTerminalWidget::StartDisplayingLines,
        0.1f,
        false,
        3.f
    );
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
            if(BinaryGuessActivated)
            {
                if ((Char == '0' || Char == '1') && CurrentInputLine.Len() < BinaryLength)
                {
                    CurrentInputLine.AppendChar(Char);
                    CommitGuessNum();
                    UpdateDisplay();
                }
            }
            else{
                CurrentInputLine.AppendChar(Char);
                UpdateDisplay();
            }
            }
 
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UTerminalWidget::CommitGuessNum()
{
    if(CurrentInputLine.Len() != BinaryLength)
    {
        return;
    }
    CurrentText += CurrentInputLine + TEXT("\n");
    AddToGuessHistory(CurrentInputLine);

    CurrentInputLine.Empty();
    UpdateDisplay();
}

void UTerminalWidget::CommitInput()
{
    // 防止越界
    if(NextCommandIndex >= CommandSequence.Num())
    {
        return;
    }

    // 目前应当输入的命令
    const FString& Expected = CommandSequence[NextCommandIndex];
    
    // 先把输入行追加到 CurrentText
    CurrentText += CurrentInputLine + TEXT("\n");
    UE_LOG(LogTemp, Display, TEXT("now index: %d"), NextCommandIndex)

    if (CurrentInputLine.Equals(Expected))
    {

        // 优先查行为命令表
        if (CommandActionMap.Contains(CurrentInputLine))
        {
            // 执行行为（同步）
            CommandActionMap[CurrentInputLine]();
            NextCommandIndex++;
        }
        // 否则显示文字
        else if (CommandTextMap.Contains(CurrentInputLine))
        {
            // 文本命令：把对应行加入 PendingLines 并启动定时器显示
            
            PendingLines.Append(CommandTextMap[CurrentInputLine]);
            StartDisplayingLines();
            NextCommandIndex++;
        }
        else
        {
            PendingLines.Add(FString::Printf(TEXT("Unknown command: %s"), *CurrentInputLine));
            StartDisplayingLines();
        }
    }
    else
    {
        PendingLines.Add(FString::Printf(TEXT("Unknown command: %s"), *CurrentInputLine));
        StartDisplayingLines();
    }

    CurrentInputLine.Empty();
    UpdateDisplay();
}

// 用于添加单行的文本
void UTerminalWidget::AddNewLine(const FString& Line)
{
    PendingLines.Add(Line);
    StartDisplayingLines();
}

// 用于添加多行的文本
void UTerminalWidget::AddNewLines(const TArray<FString>& Lines)
{
    PendingLines.Append(Lines);
    StartDisplayingLines();
}

void UTerminalWidget::StartDisplayingLines()
{
    if (!GetWorld()) return;
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(DisplayTimerHandle);
    }
    // if (GetWorld()->GetTimerManager().IsTimerActive(DisplayTimerHandle))
    // {
    //     return;
    // }
    UE_LOG(LogTemp, Display, TEXT("StartDisplayingLines"))

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
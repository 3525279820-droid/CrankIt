#include "TerminalWidget.h"

#include <string>

#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerCamera.h"
#include "Sound/SoundWave.h"
#include "CalibrationWidget.h"
#include "UClassificationGameWidget.h"

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
        BinaryGuessActivated = false;
        PendingLines.Add(TEXT(""));
        PendingLines.Add(TEXT("Fault Successfully Fixed."));
        PendingLines.Add(TEXT(""));
        PendingLines.Add(TEXT("Error!"));
        PendingLines.Add(TEXT("Unauthorised use of BOORDLE has been detected"));
        PendingLines.Add(TEXT("Unauthorised use of BOORDLE has been detected"));
        PendingLines.Add(TEXT("please verify you are human by typing the following:"));
        PendingLines.Add(TEXT("\"I AM A HUMAN BEING\""));

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
    CommandSequence = { TEXT("CALIBRATE"), TEXT("CLASSIFY") };
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
    });
    CommandTextMap.Add(TEXT("I AM A HUMAN BEING"), {
        TEXT(""),
        TEXT("insufficient further validation required."),
        TEXT("press any key to start additional verification."),
    });
    
    CommandTextMap.Add(TEXT("LIFT QUARANTINE"),
    {
        TEXT(""),
        TEXT("ERROR!"),
        TEXT("NORTH ENTRY door is not calibrated correctly"),
        TEXT(""),
        TEXT("to calibrate door type"),
        TEXT("\"CALIBRATE NORTH ENTRY DOOR\"")
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

    // 启动分类小游戏的特殊指令
    CommandActionMap.Add(TEXT(""), [this]()
    {
        EnterClassificationGame();
    });

    CommandActionMap.Add(TEXT("CALIBRATE"), [this]()
    {
        PendingLines.Append({
            TEXT("CALIBRATE Game Started"),
        });
        StartDisplayingLines();
        EnterCalibrationGame();
    });

    CommandActionMap.Add(TEXT("UPDATE SYSTEM"), [this]()
    {
        PendingLines.Append(
            {
                TEXT(""),
                TEXT("Error!"),
                TEXT("EMERGENCY QUARANTINE IS IN EFFECT"),
                TEXT(""),
                TEXT("to access lift controls quarantine must be lifted."),
                TEXT("all doors will open upon lifting quarantine."),
                TEXT("type the following to lift quarantine"),
                TEXT("\"LIFT QUARANTINE\"")
            });
        StartDisplayingLinesProcedure(5.f);
    });

    CommandActionMap.Add(TEXT("LIFT QUARANTINE"), [this]()
    {
        PendingLines.Append(
            {
                TEXT(""),
                TEXT("ERROR!"),
                TEXT("EMERGENCY QUARANTINE IS IN EFFECT"),
                TEXT(""),
                TEXT("to access lift controls quarantine must be lifted."),
                TEXT("all doors will open upon lifting quarantine."),
                TEXT("type the following to lift quarantine"),
                TEXT("\"LIFT QUARANTINE\""),
            }
        );
        StartDisplayingLinesProcedure(5.f);
    });

    CommandActionMap.Add(TEXT("CALIBRATE NORTH ENTRY DOOR"), [this]()
    {
        PendingLines.Append(
            {
                TEXT(""),
                TEXT("Unlocking Door Please wait.."),
                TEXT(""),
            });
        StartDisplayingLinesProcedure(15.f);
        PendingLines.Append(
            {
                TEXT("Door Unlocked thank you for being"),
                TEXT("Patient"),
            });
        StartDisplayingLines();
        PendingLines.Append(
            {
                TEXT(""),
                TEXT("Error!"),
                TEXT("Low auxiliary detected."),
                TEXT("A recharge is required to operate lift safely"),
                TEXT(""),
                TEXT("press any key to continue"),
            }
        );
        StartDisplayingLinesProcedure(5.f);
        PendingLines.Append(
            {
                TEXT(""),
                TEXT("Warning this terminal has been temporarily restricted"),
                TEXT("to avoid excessive use, for YOUR SAFETY."),
                TEXT(""),
                TEXT("Terminal will be available in 60 seconds")
            }
        );
        StartDisplayingLines();
        PendingLines.Append(
            {
                TEXT("Terminal will be available in 30 seconds"),
                TEXT(""),
            }
        );
        StartDisplayingLinesProcedure(30.f);
        PendingLines.Append(
            {
                TEXT("Terminal unlocked press any key to continue"),
            }
        );
        StartDisplayingLinesProcedure(30.f);
    });
    CommandActionMap.Add(TEXT(""), [this]()
    {
        for(int32 i = 0; i < 14; ++i)
        {
            PendingLines.Add(TEXT("GOD IS DEAD"));
            StartDisplayingLinesProcedure(1.f);
        }
    });
    CommandActionMap.Add(TEXT(""), [this]()
    {
        PendingLines.Append(
            {
                TEXT(""),
                TEXT("lift operational"),
                TEXT("to ascend type"),
                TEXT("\"ASCEND\""),
            }
        );
        StartDisplayingLines();
    }
    );
    CommandActionMap.Add(TEXT("ASCEND"), [this](){
        PendingLines.Append(
            {
                TEXT("GAME OVER!"),
            }
        );
        StartDisplayingLines();
        }
    );

    //////////////////////////////////////////////////////////////////////////////////
}


void UTerminalWidget::StartDisplayingLinesProcedure(float delay)
{
    if (!GetWorld()) return;

    if (GetWorld()->GetTimerManager().IsTimerActive(DisplayTimerHandle))
    {
        return;
    }
    UE_LOG(LogTemp, Display, TEXT("rebooting"));

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

    // 如果当前处于分类小游戏中，则把输入交给小游戏处理（这里只处理 A / D / Enter 三个键）
    if (CurrentInputMode == ETerminalInputMode::ClassificationGame)
    {
        // 保留 Tab 作为兜底退出，避免输入被完全锁死
        if (Key == EKeys::Tab)
        {
            ExitClassificationGame();
            return FReply::Handled();
        }

        if (ClassificationGameWidget)
        {
            // 转发 A/D/Enter 给小游戏；其它键忽略
            if (Key == EKeys::A || Key == EKeys::Left ||
                Key == EKeys::D || Key == EKeys::Right ||
                Key == EKeys::Enter)
            {
                ClassificationGameWidget->HandleKey(Key);
            }
        }
        return FReply::Handled();
    }

    if (CurrentInputMode == ETerminalInputMode::CalibrationGame)
    {
        if (Key == EKeys::Tab)
        {
            ExitCalibrationGame();
            return FReply::Handled();
        }

        if (CalibrationWidget && Key == EKeys::Enter)
        {
            CalibrationWidget->HandleKey(Key);
        }
        return FReply::Handled();
    }

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
    // 清空终端
    ClearTerminal();

    // 目前应当输入的命令
    const FString& Expected = CommandSequence[NextCommandIndex];
    
    // 先把输入行追加到 CurrentText
    CurrentText += CurrentInputLine + TEXT("\n");
    UE_LOG(LogTemp, Display, TEXT("now index: %d"), NextCommandIndex);

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

    UE_LOG(LogTemp, Display, TEXT("StartDisplayingLines"));

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

// ================= 分类小游戏模式切换 =================

void UTerminalWidget::EnterClassificationGame()
{
    if (!ClassificationGameWidget)
    {
        PendingLines.Add(TEXT("ClassificationGameWidget is not bound. Please check widget name in UMG."));
        StartDisplayingLines();
        bInClassificationGame = false;
        CurrentInputMode = ETerminalInputMode::Terminal;
        return;
    }

    // 切换到分类小游戏输入模式
    bInClassificationGame = true;
    CurrentInputMode = ETerminalInputMode::ClassificationGame;

    // 清空当前终端输入行，避免残留
    CurrentInputLine.Empty();
    UpdateDisplay();

    // 这里可以追加一行提示信息到终端（如果你希望在进入小游戏前在终端上写一行文字）
    // PendingLines.Add(TEXT("Classification game started."));
    // StartDisplayingLines();

    // 实际项目中，你可以在这里显示分类小游戏的 Widget（比如通过蓝图绑定的子 Widget）
    ClassificationGameWidget->SetVisibility(ESlateVisibility::Visible);

        // 生成 3x8 的默认测试数据（先用文字 ID 替代图片）
        TArray<FClassificationItem> DefaultItems;
        DefaultItems.Reserve(24);
        for (int32 i = 0; i < 24; ++i)
        {
            FClassificationItem Item;
            Item.Id = FName(*FString::Printf(TEXT("Item_%02d"), i + 1));
            if (i < 8)
            {
                Item.CorrectCategory = EClassificationCategory::Animal;
            }
            else if (i < 16)
            {
                Item.CorrectCategory = EClassificationCategory::Fruit;
            }
            else
            {
                Item.CorrectCategory = EClassificationCategory::Sport;
            }
            DefaultItems.Add(Item);
        }
    ClassificationGameWidget->StartGame(DefaultItems);

    // 绑定结束事件（避免重复绑定）
    ClassificationGameWidget->OnGameFinished.RemoveAll(this);
    ClassificationGameWidget->OnGameFinished.AddDynamic(this, &UTerminalWidget::HandleClassificationGameFinished);
}

void UTerminalWidget::HandleClassificationGameFinished(bool bAllCorrect)
{
	if (bAllCorrect)
	{
		PendingLines.Add(TEXT("All classifications correct."));
	}
	else
	{
		PendingLines.Add(TEXT("Classification game ended."));
	}
	ExitClassificationGame();
}

void UTerminalWidget::ExitClassificationGame()
{
    // 小游戏结束后，由小游戏调用此函数，恢复终端输入模式
    bInClassificationGame = false;
    CurrentInputMode = ETerminalInputMode::Terminal;

    // 恢复终端的输入行
    CurrentInputLine.Empty();
    UpdateDisplay();

    // 可以在终端中提示小游戏结束
    PendingLines.Add(TEXT("Classification game finished. Back to terminal."));
    StartDisplayingLines();

    if (ClassificationGameWidget)
    {
        ClassificationGameWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UTerminalWidget::EnterCalibrationGame()
{
    if (!CalibrationWidget)
    {
        PendingLines.Add(TEXT("CalibrationWidget is not bound. Please check widget name in UMG."));
        StartDisplayingLines();
        CurrentInputMode = ETerminalInputMode::Terminal;
        return;
    }

    bInClassificationGame = false;
    CurrentInputMode = ETerminalInputMode::CalibrationGame;
    CurrentInputLine.Empty();
    UpdateDisplay();

    CalibrationWidget->SetVisibility(ESlateVisibility::Visible);

    // 终端里若 TextBlock 铺满 Canvas，后添加的子项默认 ZOrder 可能更低，导致小游戏被完全挡住。
    if (UPanelSlot* PanelSlot = CalibrationWidget->Slot)
    {
        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(PanelSlot))
        {
            CanvasSlot->SetZOrder(1000);
        }
    }

    CalibrationWidget->InvalidateLayoutAndVolatility();
    CalibrationWidget->OnGameFinished.RemoveAll(this);
    CalibrationWidget->OnGameFinished.AddDynamic(this, &UTerminalWidget::HandleCalibrationGameFinished);
    CalibrationWidget->StartGame();
}

void UTerminalWidget::HandleCalibrationGameFinished(bool bWon)
{
    if (bWon)
    {
        PendingLines.Add(TEXT(""));
        PendingLines.Add(TEXT("HUMAN BEING verified"));
        PendingLines.Add(TEXT(""));
        PendingLines.Add(TEXT("Error!"));
        PendingLines.Add(TEXT("An update is required to continue"));
        PendingLines.Add(TEXT(""));
        PendingLines.Add(TEXT("type the following to start the update"));
        PendingLines.Add(TEXT("\"UPDATE SYSTEM\""));
    }
    else
    {
        PendingLines.Add(TEXT("Calibration game finished."));
    }
    ExitCalibrationGame();
}

void UTerminalWidget::ExitCalibrationGame()
{
    CurrentInputMode = ETerminalInputMode::Terminal;
    CurrentInputLine.Empty();
    UpdateDisplay();

    PendingLines.Add(TEXT("Calibration game finished. Back to terminal."));
    StartDisplayingLines();

    if (CalibrationWidget)
    {
        CalibrationWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
}


void UTerminalWidget::UpdateDisplay()
{
    if (TerminalText)
    {
        TerminalText->SetText(FText::FromString(CurrentText + CurrentInputLine));
    }
}

void UTerminalWidget::ClearTerminal()
{
}

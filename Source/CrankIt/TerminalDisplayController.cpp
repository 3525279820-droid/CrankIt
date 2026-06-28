#include "TerminalDisplayController.h"

#include "Components/TextBlock.h"
#include "TerminalWidget.h"
#include "TimerManager.h"

// 绑定所属 Widget 与 TextBlock，后续显示与定时器均依赖二者
void UTerminalDisplayController::Initialize(UTerminalWidget* InOwner, UTextBlock* InTerminalText)
{
	OwnerWidget = InOwner;
	TerminalText = InTerminalText;
}

// 从 OwnerWidget 取 World，UObject 子对象本身无 World 上下文
UWorld* UTerminalDisplayController::GetWorld() const
{
	return OwnerWidget ? OwnerWidget->GetWorld() : nullptr;
}

// 延迟启动逐行显示，结束后执行 OnProcedureComplete
void UTerminalDisplayController::StartDisplayingLinesProcedure(float Delay, TFunction<void()> OnProcedureComplete)
{
	if (!GetWorld())
	{
		return;
	}

	if (GetWorld()->GetTimerManager().IsTimerActive(DisplayTimerHandle))
	{
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("rebooting"));

	ProcedureOnComplete = MoveTemp(OnProcedureComplete);

	// 与原先 TerminalWidget 一致：Delay 参数未用于初始等待，固定 3 秒后再 StartDisplayingLines
	GetWorld()->GetTimerManager().SetTimer(
		DisplayTimerHandle,
		this,
		&UTerminalDisplayController::StartDisplayingLines,
		0.1f,
		false,
		3.f
	);
}

// 启动逐行输出定时器，若有 Procedure 回调则先执行
void UTerminalDisplayController::StartDisplayingLines()
{
	if (!GetWorld())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(DisplayTimerHandle);

	UE_LOG(LogTemp, Display, TEXT("StartDisplayingLines"));

	TFunction<void()> Callback = MoveTemp(ProcedureOnComplete);
	ProcedureOnComplete = nullptr;

	if (Callback)
	{
		Callback();
	}

	// 按 Interval 循环取出 PendingLines 逐行显示
	GetWorld()->GetTimerManager().SetTimer(
		DisplayTimerHandle,
		this,
		&UTerminalDisplayController::DisplayNextLine,
		Interval,
		true
	);
}

// 从 PendingLines 取出一行追加到 CurrentText 并刷新 UI
void UTerminalDisplayController::DisplayNextLine()
{
	if (PendingLines.Num() == 0)
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(DisplayTimerHandle);
		}
		return;
	}

	const FString Line = PendingLines[0];
	PendingLines.RemoveAt(0);

	CurrentText += Line + TEXT("\n");
	UpdateDisplay();
}

// 将 CurrentText + 当前输入行写入 TextBlock
void UTerminalDisplayController::UpdateDisplay()
{
	if (TerminalText)
	{
		TerminalText->SetText(FText::FromString(CurrentText + CurrentInputLine));
	}
}

// 清空屏幕内容与 TextBlock
void UTerminalDisplayController::ClearTerminal()
{
	if (TerminalText)
	{
		TerminalText->SetText(FText::GetEmpty());
	}
	CurrentText.Empty();
}

// 追加一行并立即开始显示
void UTerminalDisplayController::AddNewLine(const FString& Line)
{
	AddPendingLine(Line);
	StartDisplayingLines();
}

// 追加多行并立即开始显示
void UTerminalDisplayController::AddNewLines(const TArray<FString>& Lines)
{
	AppendPendingLines(Lines);
	StartDisplayingLines();
}

// 压入待显示队列，不自动启动定时器
void UTerminalDisplayController::AddPendingLine(const FString& Line)
{
	PendingLines.Add(Line);
}

// 批量压入待显示队列
void UTerminalDisplayController::AppendPendingLines(const TArray<FString>& Lines)
{
	PendingLines.Append(Lines);
}

// 清空待显示队列
void UTerminalDisplayController::ClearPendingLines()
{
	PendingLines.Empty();
}

// 移除满足条件的待显示行（如错误提示行）
void UTerminalDisplayController::RemovePendingLinesIf(TFunctionRef<bool(const FString&)> Predicate)
{
	PendingLines.RemoveAll([&Predicate](const FString& Line)
	{
		return Predicate(Line);
	});
}

// 停止逐行显示定时器
void UTerminalDisplayController::ClearDisplayTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DisplayTimerHandle);
	}
}

// 查询逐行显示定时器是否在运行
bool UTerminalDisplayController::IsDisplayTimerActive() const
{
	if (const UWorld* World = GetWorld())
	{
		return World->GetTimerManager().IsTimerActive(DisplayTimerHandle);
	}
	return false;
}

// 一次性定时器，复用 DisplayTimerHandle（如 BOORDLE 重置倒计时）
void UTerminalDisplayController::SetOneShotTimer(float DelaySeconds, TFunction<void()> Callback)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DisplayTimerHandle,
			FTimerDelegate::CreateLambda([MovedCallback = MoveTemp(Callback)]() mutable
			{
				if (MovedCallback)
				{
					MovedCallback();
				}
			}),
			DelaySeconds,
			false
		);
	}
}

void UTerminalDisplayController::SetCurrentInputLine(const FString& Line)
{
	CurrentInputLine = Line;
}

void UTerminalDisplayController::AppendCharToInputLine(TCHAR Char)
{
	CurrentInputLine.AppendChar(Char);
}

void UTerminalDisplayController::RemoveLastInputChar()
{
	if (!CurrentInputLine.IsEmpty())
	{
		CurrentInputLine.RemoveAt(CurrentInputLine.Len() - 1);
	}
}

void UTerminalDisplayController::ClearInputLine()
{
	CurrentInputLine.Empty();
}

void UTerminalDisplayController::AppendToCurrentText(const FString& Text)
{
	CurrentText += Text;
}

bool UTerminalDisplayController::CurrentTextEndsWith(const FString& Suffix) const
{
	return CurrentText.EndsWith(Suffix);
}

// 撤销已提交到屏幕末尾的错误输入/错误提示文本
void UTerminalDisplayController::RemoveCurrentTextSuffix(const FString& Suffix)
{
	if (!Suffix.IsEmpty() && CurrentText.EndsWith(Suffix))
	{
		CurrentText.RemoveAt(CurrentText.Len() - Suffix.Len(), Suffix.Len());
	}
}

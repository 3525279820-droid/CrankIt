#pragma once

// Terminal/MiniGames — BOORDLE 二进制猜数逻辑（无 UI / 无 UObject）

#include "CoreMinimal.h"

/** 单次猜测提交后，终端需依次处理的输出事件 */
struct FBoardleGuessOutcome
{
	enum class EKind : uint8
	{
		GuessSummary,
		RoundWon,
		RoundLost,
	};

	EKind Kind = EKind::GuessSummary;
	FString Text;
	bool bEndSession = false;
	bool bScheduleReset = false;
};

class FBoardleGameController
{
public:
	void Configure(int32 InBinaryLength, int32 InMaxGuesses);

	/** 生成新目标串并重置计数（Construct 或新一轮） */
	void ResetRound();

	/** BOORDLE 命令触发：进入 0/1 输入模式（目标串已在 ResetRound 中生成） */
	void StartSession();

	void EndSession();

	bool IsActive() const { return bActive; }
	int32 GetBinaryLength() const { return BinaryLength; }

	/** 玩家凑满 BinaryLength 位后提交；返回需写入终端的输出序列 */
	TArray<FBoardleGuessOutcome> SubmitGuess(const FString& Guess);

private:
	void GenerateTarget();

	int32 BinaryLength = 6;
	int32 MaxGuesses = 7;
	int32 CurrentGuessCount = 0;
	FString NewTarget;
	TArray<FString> GuessHistory;
	bool bActive = false;
};

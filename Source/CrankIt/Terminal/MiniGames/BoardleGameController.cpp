#include "BoardleGameController.h"

void FBoardleGameController::Configure(int32 InBinaryLength, int32 InMaxGuesses)
{
	BinaryLength = InBinaryLength;
	MaxGuesses = InMaxGuesses;
}

void FBoardleGameController::GenerateTarget()
{
	NewTarget.Empty();
	for (int32 i = 0; i < BinaryLength; ++i)
	{
		NewTarget += FMath::RandBool() ? TEXT("1") : TEXT("0");
	}
}

void FBoardleGameController::ResetRound()
{
	GuessHistory.Empty();
	CurrentGuessCount = 0;
	GenerateTarget();
}

void FBoardleGameController::StartSession()
{
	bActive = true;
}

void FBoardleGameController::EndSession()
{
	bActive = false;
}

TArray<FBoardleGuessOutcome> FBoardleGameController::SubmitGuess(const FString& Guess)
{
	TArray<FBoardleGuessOutcome> Outcomes;
	if (!bActive || Guess.Len() != BinaryLength)
	{
		return Outcomes;
	}

	GuessHistory.Add(Guess);
	++CurrentGuessCount;

	int32 MatchCount = 0;
	for (int32 i = 0; i < BinaryLength; ++i)
	{
		if (Guess[i] == NewTarget[i])
		{
			++MatchCount;
		}
	}

	FBoardleGuessOutcome Summary;
	Summary.Kind = FBoardleGuessOutcome::EKind::GuessSummary;
	Summary.Text = FString::Printf(
		TEXT("Guess %d: %s  (matching bits: %d)"),
		CurrentGuessCount,
		*Guess,
		MatchCount);
	Outcomes.Add(Summary);

	if (MatchCount == BinaryLength)
	{
		bActive = false;
		FBoardleGuessOutcome Won;
		Won.Kind = FBoardleGuessOutcome::EKind::RoundWon;
		Won.bEndSession = true;
		Outcomes.Add(Won);
		return Outcomes;
	}

	if (CurrentGuessCount >= MaxGuesses)
	{
		FBoardleGuessOutcome Lost;
		Lost.Kind = FBoardleGuessOutcome::EKind::RoundLost;
		Lost.Text = FString::Printf(TEXT("Target decimal was: %s"), *NewTarget);
		Lost.bScheduleReset = true;
		Outcomes.Add(Lost);
	}

	return Outcomes;
}

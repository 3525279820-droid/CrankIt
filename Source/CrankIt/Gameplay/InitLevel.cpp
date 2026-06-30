// Fill out your copyright notice in the Description page of Project Settings.


#include "InitLevel.h"

#include "CrankItIntroFlowSubsystem.h"
#include "CrankItNarrativeSubsystem.h"

AInitLevel::AInitLevel()
{
}

// 获取本关卡 IntroFlow 子系统
UCrankItIntroFlowSubsystem* AInitLevel::GetIntroFlow() const
{
	if (UWorld* World = GetWorld())
	{
		return World->GetSubsystem<UCrankItIntroFlowSubsystem>();
	}
	return nullptr;
}

void AInitLevel::BeginPlay()
{
	Super::BeginPlay();

	// 将 GameMode 上配置的 Data Asset 注入叙事子系统
	if (UWorld* World = GetWorld())
	{
		if (UCrankItNarrativeSubsystem* Narrative = World->GetSubsystem<UCrankItNarrativeSubsystem>())
		{
			Narrative->SetNarrativeData(NarrativeData);
			Narrative->SetTerminalCommandData(TerminalCommandData);
		}

		// 开场 / 教程 / 过场流程交由 IntroFlowSubsystem（配置仍在本 GameMode）
		if (UCrankItIntroFlowSubsystem* IntroFlow = World->GetSubsystem<UCrankItIntroFlowSubsystem>())
		{
			IntroFlow->StartIntroFlow(this);
		}
	}
}

// 以下公开 API 转发至 UCrankItIntroFlowSubsystem，Blueprint 与 MineConsole 等调用路径不变

void AInitLevel::OnTutorialClosed()
{
	if (UCrankItIntroFlowSubsystem* IntroFlow = GetIntroFlow())
	{
		IntroFlow->OnTutorialClosed();
	}
}

void AInitLevel::ShowKeyPrompt()
{
	if (UCrankItIntroFlowSubsystem* IntroFlow = GetIntroFlow())
	{
		IntroFlow->ShowKeyPrompt();
	}
}

void AInitLevel::PlaySequence(FName SequenceTag, bool bLoop)
{
	if (UCrankItIntroFlowSubsystem* IntroFlow = GetIntroFlow())
	{
		IntroFlow->PlaySequence(SequenceTag, bLoop);
	}
}

void AInitLevel::SetFirstComputerScreenText()
{
	if (UCrankItIntroFlowSubsystem* IntroFlow = GetIntroFlow())
	{
		IntroFlow->SetFirstComputerScreenText();
	}
}

void AInitLevel::ShowSkipTutorial()
{
	if (UCrankItIntroFlowSubsystem* IntroFlow = GetIntroFlow())
	{
		IntroFlow->ShowSkipTutorial();
	}
}

void AInitLevel::ShowTutorial()
{
	if (UCrankItIntroFlowSubsystem* IntroFlow = GetIntroFlow())
	{
		IntroFlow->ShowTutorial();
	}
}

void AInitLevel::PrepareLevel()
{
	if (UCrankItIntroFlowSubsystem* IntroFlow = GetIntroFlow())
	{
		IntroFlow->PrepareLevel();
	}
}

void AInitLevel::StopIntroCutsceneAndReturnToGame()
{
	if (UCrankItIntroFlowSubsystem* IntroFlow = GetIntroFlow())
	{
		IntroFlow->StopIntroCutsceneAndReturnToGame();
	}
}

void AInitLevel::TutorialSkipped()
{
	if (UCrankItIntroFlowSubsystem* IntroFlow = GetIntroFlow())
	{
		IntroFlow->TutorialSkipped();
	}
}

void AInitLevel::TutorialNotSkipped()
{
	if (UCrankItIntroFlowSubsystem* IntroFlow = GetIntroFlow())
	{
		IntroFlow->TutorialNotSkipped();
	}
}

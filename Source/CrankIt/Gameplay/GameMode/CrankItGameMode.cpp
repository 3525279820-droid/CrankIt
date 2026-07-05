#include "CrankItGameMode.h"

#include "CrankItIntroFlowSubsystem.h"
#include "CrankItNarrativeSubsystem.h"

ACrankItGameMode::ACrankItGameMode()
{
}

// 注入 Narrative Data Asset，并启动 IntroFlow（隧道停止、过场、Skip 教程等）
void ACrankItGameMode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("CrankItGameMode: BeginPlay"));
	if (UWorld* World = GetWorld())
	{
		if (UCrankItNarrativeSubsystem* Narrative = World->GetSubsystem<UCrankItNarrativeSubsystem>())
		{
			Narrative->SetNarrativeData(NarrativeData);
			Narrative->SetTerminalCommandData(TerminalCommandData);
		}

		if (UCrankItIntroFlowSubsystem* IntroFlow = World->GetSubsystem<UCrankItIntroFlowSubsystem>())
		{
			UE_LOG(LogTemp, Log, TEXT("CrankItGameMode: 启动 IntroFlow"));
			IntroFlow->StartIntroFlow(this);
		}
	}
}

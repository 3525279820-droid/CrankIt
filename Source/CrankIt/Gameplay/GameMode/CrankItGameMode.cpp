#include "CrankItGameMode.h"

#include "CrankItIntroFlowSubsystem.h"
#include "CrankItNarrativeSubsystem.h"
#include "CrankItAudioService.h"

ACrankItGameMode::ACrankItGameMode()
{
}

// 注入 Narrative Data Asset，并启动 IntroFlow（隧道停止、过场、Skip 教程等）
void ACrankItGameMode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("CrankItGameMode: BeginPlay"));

	if (UCrankItAudioService* Audio = UCrankItAudioService::Get(this))
	{
		LevelBackgroundMusicHandle = Audio->Play2DLoop(LevelBackgroundMusicSound);
	}

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

void ACrankItGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UCrankItAudioService* Audio = UCrankItAudioService::Get(this))
	{
		Audio->Stop(LevelBackgroundMusicHandle);
	}
	Super::EndPlay(EndPlayReason);
}

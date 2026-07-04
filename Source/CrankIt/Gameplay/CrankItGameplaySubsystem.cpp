#include "CrankItGameplaySubsystem.h"

#include "CrankItActorRegistry.h"
#include "MineConsole.h"

void UCrankItGameplaySubsystem::NotifyPostEMPTutorialFinished()
{
	OnPostEMPTutorialFinished.Broadcast();
}

void UCrankItGameplaySubsystem::SetTutorialSkipped(bool bSkipped)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (UCrankItActorRegistry* Registry = World->GetSubsystem<UCrankItActorRegistry>())
	{
		if (AMineConsole* Console = Registry->GetMineConsole())
		{
			Console->SetTutorialSkipped(bSkipped);
		}
	}
}

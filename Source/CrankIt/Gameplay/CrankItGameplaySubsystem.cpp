#include "CrankItGameplaySubsystem.h"

#include "CrankItActorRegistry.h"
#include "MineConsole.h"

// 广播 OnPostEMPTutorialFinished，供 IntroFlow::PrepareLevel 等订阅者响应
void UCrankItGameplaySubsystem::NotifyPostEMPTutorialFinished()
{
	OnPostEMPTutorialFinished.Broadcast();
}

// 经 ActorRegistry 找到 AMineConsole 并标记教程已跳过
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

#pragma once

// 主关卡 GameMode：编辑器可配项（叙事 Data Asset、教程、过场 Tag）；BeginPlay 注入子系统并启动 IntroFlow。
// 开场 / 教程 / 过场具体逻辑见 Gameplay/Subsystems/CrankItIntroFlowSubsystem。

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CrankItAudioService.h"
#include "CrankItGameMode.generated.h"

class UCrankItNarrativeData;
class UCrankItTerminalCommandData;
class USDTutorialWidget;
class USoundBase;

UCLASS()
class CRANKIT_API ACrankItGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACrankItGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// 教程 Widget 类列表（须继承 USDTutorialWidget）；IntroFlow 按序显示，关闭后索引自增
	UPROPERTY(EditAnywhere, Category = "Tutorial")
	TArray<TSubclassOf<USDTutorialWidget>> TutorialWidgetClasses;

	// 开场下降结束后、播放 Intro 过场前的等待秒数（IntroFlow::OnDescendTimerFired）
	UPROPERTY(EditAnywhere, Category = "Cinematic")
	float DesendTime = 10.f;

	// BeginPlay 定时器到期后播放的 Level Sequence Actor Tag
	UPROPERTY(EditAnywhere, Category = "Cinematic")
	FName IntroSequenceActorTag;

	// 字幕剧本 Primary Data Asset；TrackId 见 Narrative/CrankItNarrativeIds.h
	UPROPERTY(EditDefaultsOnly, Category = "Narrative")
	TObjectPtr<UCrankItNarrativeData> NarrativeData;

	// 终端命令顺序、输出块与 ActionId 映射；供 UCrankItNarrativeSubsystem / TerminalWidget 读取
	UPROPERTY(EditDefaultsOnly, Category = "Narrative")
	TObjectPtr<UCrankItTerminalCommandData> TerminalCommandData;

	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<USoundBase> LevelBackgroundMusicSound = nullptr;

private:
	FCrankItSoundHandle LevelBackgroundMusicHandle;
};

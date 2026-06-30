#pragma once

// 统一从 Data Asset 读取字幕与终端文案，并转发给 USubtitleSubsystem

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SubtitleSubsystem.h"
#include "TerminalCommandRouter.h"
#include "CrankItNarrativeSubsystem.generated.h"

class UCrankItNarrativeData;
class UCrankItTerminalCommandData;

UCLASS()
class CRANKIT_API UCrankItNarrativeSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void SetNarrativeData(UCrankItNarrativeData* InData);
	void SetTerminalCommandData(UCrankItTerminalCommandData* InData);
	void RefreshDataFromGameMode() const;

	void PlaySubtitleTrack(FName TrackId, TFunction<void()> OnComplete = TFunction<void()>());

	UFUNCTION(BlueprintCallable, Category = "Narrative")
	bool GetSubtitleTrackLines(FName TrackId, TArray<FCrankItSubtitleLine>& OutLines) const;

	UFUNCTION(BlueprintCallable, Category = "Narrative")
	bool GetTerminalOutputLines(FName BlockId, TArray<FString>& OutLines) const;

	bool ApplyTerminalCommandData(TArray<FString>& OutSequence, TMap<FString, TArray<FString>>& OutTextMap) const;

	// 填充 CommandRouter 配置（顺序、文本表、ActionId）；供 TerminalWidget::NativeConstruct 调用
	bool ApplyTerminalCommandRouterConfig(FTerminalCommandRouterConfig& OutConfig) const;

	UCrankItNarrativeData* GetNarrativeData() const { return NarrativeData; }
	UCrankItTerminalCommandData* GetTerminalCommandData() const { return TerminalCommandData; }

private:
	// mutable：允许在 const 查询中懒加载 GameMode 上的 Data Asset
	mutable UPROPERTY()
	TObjectPtr<UCrankItNarrativeData> NarrativeData;

	mutable UPROPERTY()
	TObjectPtr<UCrankItTerminalCommandData> TerminalCommandData;
};

#pragma once

// 全局 HUD Widget 生命周期（字幕等）；避免 Pawn 直接 CreateWidget

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrankItUIService.generated.h"

class APlayerController;
class USubtitleWidget;

UCLASS()
class CRANKIT_API UCrankItUIService : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 若尚未创建则 CreateWidget 并 AddToPlayerScreen；已存在则直接返回（通常由 APlayerCamera::BeginPlay 调用）
	USubtitleWidget* EnsureSubtitleWidget(
		APlayerController* PC,
		TSubclassOf<USubtitleWidget> WidgetClass,
		int32 ZOrder = 200);

	USubtitleWidget* GetSubtitleWidget() const { return SubtitleWidget.Get(); }

private:
	TWeakObjectPtr<USubtitleWidget> SubtitleWidget;
};

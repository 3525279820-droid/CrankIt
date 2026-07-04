#include "CrankItUIService.h"

#include "SubtitleWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

// 创建或复用字幕 HUD Widget，并挂到 PlayerScreen（ZOrder 默认 200）
USubtitleWidget* UCrankItUIService::EnsureSubtitleWidget(
	APlayerController* PC,
	TSubclassOf<USubtitleWidget> WidgetClass,
	int32 ZOrder)
{
	if (!PC)
	{
		return nullptr;
	}

	if (SubtitleWidget.IsValid())
	{
		return SubtitleWidget.Get();
	}

	USubtitleWidget* Created = WidgetClass
		? CreateWidget<USubtitleWidget>(PC, WidgetClass)
		: CreateWidget<USubtitleWidget>(PC);

	if (Created)
	{
		Created->AddToPlayerScreen(ZOrder);
		SubtitleWidget = Created;
	}

	return Created;
}

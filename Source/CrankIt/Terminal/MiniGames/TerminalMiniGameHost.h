#pragma once

// Terminal/MiniGames — 分类 / 校准小游戏：模式切换、按键转发、结束回调

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TerminalSharedTypes.h"
#include "TerminalMiniGameHost.generated.h"

class UTerminalWidget;
class UTerminalDisplayController;
class UClassificationGameWidget;
class UCalibrationWidget;

UCLASS()
class CRANKIT_API UTerminalMiniGameHost : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(
		UTerminalWidget* InHost,
		UTerminalDisplayController* InDisplay,
		UClassificationGameWidget* InClassificationWidget,
		UCalibrationWidget* InCalibrationWidget);

	ETerminalInputMode GetInputMode() const { return CurrentInputMode; }

	// 分类 / 校准模式下转发按键；返回 true 表示已消费
	bool RouteKey(const FKey& Key);

	UFUNCTION(BlueprintCallable, Category = "Terminal|MiniGame")
	void EnterClassificationGame();

	UFUNCTION(BlueprintCallable, Category = "Terminal|MiniGame")
	void ExitClassificationGame();

	void EnterCalibrationGame(TFunction<void(bool bWon)> OnComplete = TFunction<void(bool)>());

	UFUNCTION(BlueprintCallable, Category = "Terminal|MiniGame")
	void ExitCalibrationGame();

	UFUNCTION()
	void HandleClassificationGameFinished(bool bAllCorrect);

	UFUNCTION()
	void HandleCalibrationGameFinished(bool bWon);

	void ExitCalibrationGameSilently();

private:
	UPROPERTY()
	TObjectPtr<UTerminalWidget> Host;

	UPROPERTY()
	TObjectPtr<UTerminalDisplayController> Display;

	UPROPERTY()
	TObjectPtr<UClassificationGameWidget> ClassificationGameWidget;

	UPROPERTY()
	TObjectPtr<UCalibrationWidget> CalibrationWidget;

	ETerminalInputMode CurrentInputMode = ETerminalInputMode::Terminal;

	TFunction<void(bool bWon)> CalibrationCompleteCallback;
};

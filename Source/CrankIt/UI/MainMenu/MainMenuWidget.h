#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UVerticalBox;
class UWidget;

UCLASS(BlueprintType, Blueprintable)
class CRANKIT_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** Option 子菜单显隐控制（按需在蓝图事件里调用）。 */
	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void SetOptionSubMenuVisible(bool bVisible);

	/** Credits 子菜单显隐控制（按需在蓝图事件里调用）。 */
	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void SetCreditsSubMenuVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void ToggleOptionSubMenu();

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void ToggleCreditsSubMenu();

	/** 预留按钮逻辑入口，默认空实现，建议在蓝图里重写。 */
	UFUNCTION(BlueprintImplementableEvent, Category = "MainMenu|Actions")
	void HandleStartClicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "MainMenu|Actions")
	void HandleOptionClicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "MainMenu|Actions")
	void HandleCreditsClicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "MainMenu|Actions")
	void HandleExitClicked();

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MainMenu|Widgets")
	UButton* StartButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MainMenu|Widgets")
	UButton* OptionButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MainMenu|Widgets")
	UButton* CreditsButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MainMenu|Widgets")
	UButton* ExitButton = nullptr;

	/** Option 按钮对应子菜单根节点（内容由蓝图自行创建）。 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MainMenu|Widgets")
	UVerticalBox* OptionSubMenu = nullptr;

	/** Credits 按钮对应子菜单根节点（内容由蓝图自行创建）。 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MainMenu|Widgets")
	UVerticalBox* CreditsSubMenu = nullptr;

private:
	UFUNCTION()
	void OnStartButtonClicked();

	UFUNCTION()
	void OnOptionButtonClicked();

	UFUNCTION()
	void OnCreditsButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();

	void SetSubMenuVisibility(UWidget* MenuWidget, bool bVisible);
};

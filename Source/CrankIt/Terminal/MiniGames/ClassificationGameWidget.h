// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ClassificationGameWidget.generated.h"

class UTextBlock;

UENUM(BlueprintType)
enum class EClassificationCategory : uint8
{
	Animal UMETA(DisplayName = "animal"),
	Fruit UMETA(DisplayName = "fruit"),
	Sport UMETA(DisplayName = "sport"),
};

USTRUCT(BlueprintType)
struct FClassificationItem
{
	GENERATED_BODY()

	// 你后续可以替换为 Texture/Material/SoftObjectPath 等
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Classification")
	FName Id = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Classification")
	EClassificationCategory CorrectCategory = EClassificationCategory::Animal;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClassificationGameFinished, bool, bAllCorrect);

/**
 * 3x8 分类小游戏 Widget（最小可用逻辑版本）
 */
UCLASS()
class CRANKIT_API UClassificationGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 由外部（TerminalWidget）转发按键调用
	UFUNCTION(BlueprintCallable, Category="Classification|Input")
	void HandleKey(const FKey& Key);

	// 初始化/重开一局（Items 不够 24 个时会尽量填充）
	UFUNCTION(BlueprintCallable, Category="Classification")
	void StartGame(const TArray<FClassificationItem>& InItems);

	// 让蓝图实现具体的 UI 刷新（图片光标/选项光标/错误提示等）
	UFUNCTION(BlueprintImplementableEvent, Category="Classification|UI")
	void BP_OnStateChanged(int32 InImageCursorIndex, EClassificationCategory InOptionCursor);

	UFUNCTION(BlueprintImplementableEvent, Category="Classification|UI")
	void BP_OnClassificationWrong(int32 InImageCursorIndex, EClassificationCategory InChosen);

	UFUNCTION(BlueprintImplementableEvent, Category="Classification|UI")
	void BP_OnClassificationCorrect(int32 InImageCursorIndex, EClassificationCategory InChosen);

	// 小游戏结束事件（TerminalWidget 监听后调用 ExitClassificationGame）
	UPROPERTY(BlueprintAssignable, Category="Classification")
	FOnClassificationGameFinished OnGameFinished;

	// 运行时状态（方便蓝图显示）
	UPROPERTY(BlueprintReadOnly, Category="Classification|State")
	int32 ImageCursorIndex = 0; // 0..23，起始 0 表示 1行1列

	UPROPERTY(BlueprintReadOnly, Category="Classification|State")
	EClassificationCategory OptionCursor = EClassificationCategory::Animal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Classification|Config")
	int32 GridRows = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Classification|Config")
	int32 GridCols = 8;

	UPROPERTY(BlueprintReadOnly, Category="Classification|State")
	TArray<FClassificationItem> Items;

	UPROPERTY(BlueprintReadOnly, Category="Classification|State")
	TArray<bool> bClassifiedCorrectly;

private:
	void MoveOptionLeft();
	void MoveOptionRight();
	void ConfirmChoice();
	bool IsAllCorrect() const;
	void BuildRuntimeTextUI();
	void RefreshRuntimeTextUI();
	FString CategoryToString(EClassificationCategory Category) const;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> CellTextBlocks;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> OptionTextBlocks;
};

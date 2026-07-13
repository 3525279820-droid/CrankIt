#pragma once

// Terminal/MiniGames — 3×8 分类小游戏：从 Content 目录随机抽图，网格用 UImage 等大显示

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Fonts/SlateFontInfo.h"
#include "ClassificationGameWidget.generated.h"

class UBorder;
class UImage;
class UTexture2D;
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

	UPROPERTY(BlueprintReadOnly, Category = "Classification")
	TObjectPtr<UTexture2D> Image = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Classification")
	EClassificationCategory CorrectCategory = EClassificationCategory::Animal;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClassificationGameFinished, bool, bAllCorrect);

UCLASS()
class CRANKIT_API UClassificationGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 由 TerminalMiniGameHost 转发 A / D / Enter
	UFUNCTION(BlueprintCallable, Category = "Classification|Input")
	void HandleKey(const FKey& Key);

	// 扫描 ImageContentPath 并随机开局（GridRows × GridCols 张）
	UFUNCTION(BlueprintCallable, Category = "Classification")
	void StartGame();

	UFUNCTION(BlueprintImplementableEvent, Category = "Classification|UI")
	void BP_OnStateChanged(int32 InImageCursorIndex, EClassificationCategory InOptionCursor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Classification|UI")
	void BP_OnClassificationWrong(int32 InImageCursorIndex, EClassificationCategory InChosen);

	UFUNCTION(BlueprintImplementableEvent, Category = "Classification|UI")
	void BP_OnClassificationCorrect(int32 InImageCursorIndex, EClassificationCategory InChosen);

	UPROPERTY(BlueprintAssignable, Category = "Classification")
	FOnClassificationGameFinished OnGameFinished;

	UPROPERTY(BlueprintReadOnly, Category = "Classification|State")
	int32 ImageCursorIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Classification|State")
	EClassificationCategory OptionCursor = EClassificationCategory::Animal;

	// Content 根路径，默认 /Game/Terminal/ClassificationGame；其下按 Animal / Fruit / Sport 分子目录
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification|Config")
	FString ImageContentPath = TEXT("/Game/Terminal/ClassificationGame");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification|Config")
	int32 GridRows = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification|Config")
	int32 GridCols = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification|Config", meta = (ClampMin = "16"))
	float CellImageSize = 56.f;

	/** 编辑器中指定的字体，应用于底部类别选项文字 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification|Style")
	FSlateFontInfo Font;

	UPROPERTY(BlueprintReadOnly, Category = "Classification|State")
	TArray<FClassificationItem> Items;

	UPROPERTY(BlueprintReadOnly, Category = "Classification|State")
	TArray<bool> bClassifiedCorrectly;

private:
	void MoveOptionLeft();
	void MoveOptionRight();
	void ConfirmChoice();
	bool IsAllCorrect() const;
	bool LoadRandomItemsFromContentFolder();
	void BuildRuntimeImageUI();
	void RefreshRuntimeImageUI();
	void ApplyCellImageBrush(UImage* ImageWidget, UTexture2D* Texture) const;
	void ApplyCellBorderState(UBorder* Border, bool bCursor, bool bDone) const;
	void ApplyConfiguredFont(UTextBlock* TextBlock) const;
	FString CategoryToString(EClassificationCategory Category) const;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> CellBorders;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> CellImages;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> OptionTextBlocks;
};

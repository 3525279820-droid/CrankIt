// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Fonts/SlateFontInfo.h"
#include "CalibrationWidget.generated.h"

class UBorder;
class UGridPanel;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCalibrationGameFinished, bool, bWon);

UCLASS()
class CRANKIT_API UCalibrationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category="Calibration")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category="Calibration")
	void ResetGame();

	UFUNCTION(BlueprintCallable, Category="Calibration|Input")
	void HandleKey(const FKey& Key);

	UPROPERTY(BlueprintAssignable, Category="Calibration")
	FOnCalibrationGameFinished OnGameFinished;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Calibration|Config")
	int32 GridSize = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Calibration|Config")
	float MoveInterval = 0.2f;

	/** 每行成功锁定后，移动间隔减少的秒数（数值越大加速越明显） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Calibration|Config", meta = (ClampMin = "0.0"))
	float MoveFasterPerRow = 0.02f;

	/** 移动间隔的下限，避免过快无法操作 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Calibration|Config", meta = (ClampMin = "0.01"))
	float MinMoveInterval = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Calibration|Style")
	FLinearColor MainColor = FLinearColor::Green;

	/** 编辑器中指定的字体，应用于标题、状态与行列标签 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Calibration|Style")
	FSlateFontInfo Font;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void BuildRuntimeTextUI();
	void RefreshRuntimeTextUI();
	void MoveCurrentBlockRight();
	void RestartMoveTimer();
	void ApplyConfiguredFont(UTextBlock* TextBlock) const;

	/** 当前局内实际使用的移动间隔（从 MoveInterval 开局，每成功一行递减） */
	float ActiveMoveInterval = 0.2f;
	int32 ToCellIndex(int32 LogicalRow, int32 Col) const;

	UPROPERTY(Transient)
	TObjectPtr<UGridPanel> BoardGrid = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText = nullptr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> CellBorders;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> RowLabelTexts;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> ColLabelTexts;

	FTimerHandle MoveTimerHandle;
	TArray<int32> LockedColsPerRow;
	int32 CurrentRow = 0;
	int32 CurrentMovingCol = 0;
	int32 LockedColumn = INDEX_NONE;
	bool bGameActive = false;
};

#pragma once

// 从 Content 目录扫描分类游戏纹理（Asset Registry），并按路径/文件名推断类别

#include "CoreMinimal.h"
#include "ClassificationGameWidget.h"

class UTexture2D;

class FClassificationImageLibrary
{
public:
	struct FSourceImage
	{
		TObjectPtr<UTexture2D> Texture = nullptr;
		EClassificationCategory Category = EClassificationCategory::Animal;
	};

	static bool LoadImagesFromContentPath(const FString& ContentRootPath, TArray<FSourceImage>& OutImages);
	static bool BuildRandomRound(const TArray<FSourceImage>& Pool, int32 Count, TArray<FClassificationItem>& OutItems);

private:
	static bool TryParseCategoryFromAssetPath(
		const FString& PackagePath,
		const FString& ContentRootPath,
		EClassificationCategory& OutCategory);

	static bool TryMapCategoryToken(const FString& Token, EClassificationCategory& OutCategory);
};

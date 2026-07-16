#include "ClassificationImageLibrary.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Texture2D.h"
#include "Modules/ModuleManager.h"

namespace
{
	// 规范为 Asset Registry 的 PackagePath（必须以 /Game 开头，无末尾斜杠）
	FString NormalizeContentRootPath(FString Path)
	{
		Path.TrimStartAndEndInline();
		while (Path.EndsWith(TEXT("/")))
		{
			Path.LeftChopInline(1);
		}
		if (Path.IsEmpty())
		{
			return Path;
		}
		// "ClassificationGame" / "Game/ClassificationGame" → "/Game/ClassificationGame"
		if (Path.StartsWith(TEXT("/Game")))
		{
			return Path;
		}
		if (Path.StartsWith(TEXT("Game/")))
		{
			return TEXT("/") + Path;
		}
		if (Path.StartsWith(TEXT("/")))
		{
			return FString::Printf(TEXT("/Game%s"), *Path);
		}
		return FString::Printf(TEXT("/Game/%s"), *Path);
	}
}

// 将 animal / fruit / sport 等 token 映射为枚举（大小写不敏感）
bool FClassificationImageLibrary::TryMapCategoryToken(const FString& Token, EClassificationCategory& OutCategory)
{
	const FString Lower = Token.ToLower();
	if (Lower == TEXT("animal"))
	{
		OutCategory = EClassificationCategory::Animal;
		return true;
	}
	if (Lower == TEXT("fruit"))
	{
		OutCategory = EClassificationCategory::Fruit;
		return true;
	}
	if (Lower == TEXT("sport"))
	{
		OutCategory = EClassificationCategory::Sport;
		return true;
	}
	return false;
}

// 从 Package 路径推断类别：优先子目录名（…/Animal/…），其次文件名前缀（Animal_xxx）
bool FClassificationImageLibrary::TryParseCategoryFromAssetPath(
	const FString& PackagePath,
	const FString& ContentRootPath,
	EClassificationCategory& OutCategory)
{
	const FString Root = NormalizeContentRootPath(ContentRootPath);
	if (!PackagePath.StartsWith(Root))
	{
		return false;
	}

	FString Relative = PackagePath;
	Relative.RemoveFromStart(Root);
	Relative.RemoveFromStart(TEXT("/"));

	TArray<FString> Segments;
	Relative.ParseIntoArray(Segments, TEXT("/"), true);
	if (Segments.Num() >= 2 && TryMapCategoryToken(Segments[0], OutCategory))
	{
		return true;
	}

	const FString AssetName = Segments.Num() > 0 ? Segments.Last() : FPaths::GetBaseFilename(PackagePath);
	FString Prefix;
	FString UnusedSuffix;
	if (AssetName.Split(TEXT("_"), &Prefix, &UnusedSuffix, ESearchCase::IgnoreCase, ESearchDir::FromStart)
		|| AssetName.Split(TEXT("-"), &Prefix, &UnusedSuffix, ESearchCase::IgnoreCase, ESearchDir::FromStart))
	{
		if (TryMapCategoryToken(Prefix, OutCategory))
		{
			return true;
		}
	}

	return TryMapCategoryToken(AssetName, OutCategory);
}

// 递归扫描 ContentRootPath 下全部 UTexture2D，写入 OutImages（无法推断类别的资源跳过）
bool FClassificationImageLibrary::LoadImagesFromContentPath(const FString& ContentRootPath, TArray<FSourceImage>& OutImages)
{
	OutImages.Reset();

	const FString Root = NormalizeContentRootPath(ContentRootPath);
	if (Root.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("ClassificationImageLibrary: ContentRootPath is empty."));
		return false;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	AssetRegistry.WaitForCompletion();

	FARFilter Filter;
	Filter.PackagePaths.Add(FName(*Root));
	Filter.bRecursivePaths = true;
	Filter.ClassPaths.Add(UTexture2D::StaticClass()->GetClassPathName());

	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssets(Filter, AssetDataList);

	for (const FAssetData& AssetData : AssetDataList)
	{
		const FString PackagePath = AssetData.PackageName.ToString();
		EClassificationCategory Category = EClassificationCategory::Animal;
		if (!TryParseCategoryFromAssetPath(PackagePath, Root, Category))
		{
			UE_LOG(LogTemp, Verbose, TEXT("ClassificationImageLibrary: Skip '%s' (cannot infer category)."), *PackagePath);
			continue;
		}

		UTexture2D* Texture = Cast<UTexture2D>(AssetData.GetAsset());
		if (!Texture)
		{
			Texture = LoadObject<UTexture2D>(nullptr, *AssetData.GetObjectPathString());
		}
		if (!Texture)
		{
			UE_LOG(LogTemp, Warning, TEXT("ClassificationImageLibrary: Failed to load texture '%s'."), *PackagePath);
			continue;
		}

		FSourceImage Entry;
		Entry.Texture = Texture;
		Entry.Category = Category;
		OutImages.Add(Entry);
	}

	if (OutImages.Num() == 0)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("ClassificationImageLibrary: No textures found under '%s'. Expected subfolders Animal/Fruit/Sport or prefixed filenames."),
			*Root);
		return false;
	}

	return true;
}

// 从池中无放回随机抽取 Count 项（不足则取全部），顺序即为网格从左到右、从上到下
bool FClassificationImageLibrary::BuildRandomRound(
	const TArray<FSourceImage>& Pool,
	int32 Count,
	TArray<FClassificationItem>& OutItems)
{
	OutItems.Reset();
	if (Pool.Num() == 0 || Count <= 0)
	{
		return false;
	}

	TArray<int32> Indices;
	Indices.Reserve(Pool.Num());
	for (int32 i = 0; i < Pool.Num(); ++i)
	{
		Indices.Add(i);
	}

	// Fisher–Yates 洗牌后取前 PickCount 个索引
	for (int32 i = Indices.Num() - 1; i > 0; --i)
	{
		const int32 SwapIndex = FMath::RandRange(0, i);
		if (SwapIndex != i)
		{
			Indices.Swap(i, SwapIndex);
		}
	}

	const int32 PickCount = FMath::Min(Count, Indices.Num());
	OutItems.Reserve(PickCount);
	for (int32 i = 0; i < PickCount; ++i)
	{
		const FSourceImage& Source = Pool[Indices[i]];
		FClassificationItem Item;
		Item.Image = Source.Texture;
		Item.CorrectCategory = Source.Category;
		OutItems.Add(Item);
	}

	return OutItems.Num() > 0;
}

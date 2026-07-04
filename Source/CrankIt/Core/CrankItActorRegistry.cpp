#include "CrankItActorRegistry.h"

#include "Monster.h"
#include "MineConsole.h"
#include "ComputerScreenActor.h"
#include "Tunnel.h"
#include "FlashTopLight.h"
#include "DoubleAutoDoor.h"
#include "Battery.h"
#include "Camera/CameraActor.h"
#include "EngineUtils.h"

namespace
{
	// 缓存失效时按类型重新搜索；语义与原 GetActorOfClass 取第一个一致
	template<typename T>
	void TryDiscover(UWorld* World, TWeakObjectPtr<T>& Cached)
	{
		if (Cached.IsValid() || !World)
		{
			return;
		}
		for (TActorIterator<T> It(World); It; ++It)
		{
			Cached = *It;
			break;
		}
	}
}

// 世界 BeginPlay 时填充 Actor 缓存
void UCrankItActorRegistry::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	DiscoverActors();
}

// 遍历关卡，写入各类型第一个实例的弱引用
void UCrankItActorRegistry::DiscoverActors()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TryDiscover(World, CachedMonster);
	TryDiscover(World, CachedMineConsole);
	TryDiscover(World, CachedComputerScreen);
	TryDiscover(World, CachedTunnel);
	TryDiscover(World, CachedFlashTopLight);
	TryDiscover(World, CachedDoubleAutoDoor);
	TryDiscover(World, CachedFixedCamera);
}

AMonster* UCrankItActorRegistry::GetMonster() const
{
	TryDiscover(GetWorld(), CachedMonster);
	return CachedMonster.Get();
}

AMineConsole* UCrankItActorRegistry::GetMineConsole() const
{
	TryDiscover(GetWorld(), CachedMineConsole);
	return CachedMineConsole.Get();
}

AComputerScreenActor* UCrankItActorRegistry::GetComputerScreen() const
{
	TryDiscover(GetWorld(), CachedComputerScreen);
	return CachedComputerScreen.Get();
}

ATunnel* UCrankItActorRegistry::GetTunnel() const
{
	TryDiscover(GetWorld(), CachedTunnel);
	return CachedTunnel.Get();
}

AFlashTopLight* UCrankItActorRegistry::GetFlashTopLight() const
{
	TryDiscover(GetWorld(), CachedFlashTopLight);
	return CachedFlashTopLight.Get();
}

ADoubleAutoDoor* UCrankItActorRegistry::GetDoubleAutoDoor() const
{
	TryDiscover(GetWorld(), CachedDoubleAutoDoor);
	return CachedDoubleAutoDoor.Get();
}

ACameraActor* UCrankItActorRegistry::GetFixedCamera() const
{
	TryDiscover(GetWorld(), CachedFixedCamera);
	return CachedFixedCamera.Get();
}

// 返回关卡内全部 ABattery（按需全量遍历，供 MineConsole 等使用）
void UCrankItActorRegistry::GetAllBatteries(TArray<ABattery*>& OutBatteries) const
{
	OutBatteries.Empty();
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	for (TActorIterator<ABattery> It(World); It; ++It)
	{
		if (*It)
		{
			OutBatteries.Add(*It);
		}
	}
}

// 主动注册：指定实例覆盖 Discover 结果
void UCrankItActorRegistry::RegisterMonster(AMonster* InActor)              { CachedMonster = InActor; }
void UCrankItActorRegistry::RegisterMineConsole(AMineConsole* InActor)      { CachedMineConsole = InActor; }
void UCrankItActorRegistry::RegisterComputerScreen(AComputerScreenActor* InActor) { CachedComputerScreen = InActor; }
void UCrankItActorRegistry::RegisterTunnel(ATunnel* InActor)                { CachedTunnel = InActor; }
void UCrankItActorRegistry::RegisterFlashTopLight(AFlashTopLight* InActor)  { CachedFlashTopLight = InActor; }
void UCrankItActorRegistry::RegisterDoubleAutoDoor(ADoubleAutoDoor* InActor) { CachedDoubleAutoDoor = InActor; }
void UCrankItActorRegistry::RegisterFixedCamera(ACameraActor* InActor)      { CachedFixedCamera = InActor; }

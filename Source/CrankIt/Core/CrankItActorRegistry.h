#pragma once

// 集中缓存关卡内关键 Actor，供各模块查询，取代散落的 GetActorOfClass

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrankItActorRegistry.generated.h"

class AMonster;
class AMineConsole;
class AComputerScreenActor;
class ATunnel;
class AFlashTopLight;
class ADoubleAutoDoor;
class ABattery;
class ACameraActor;

UCLASS()
class CRANKIT_API UCrankItActorRegistry : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	AMonster* GetMonster() const;
	AMineConsole* GetMineConsole() const;
	AComputerScreenActor* GetComputerScreen() const;
	ATunnel* GetTunnel() const;
	AFlashTopLight* GetFlashTopLight() const;
	ADoubleAutoDoor* GetDoubleAutoDoor() const;
	ACameraActor* GetFixedCamera() const;

	// 返回关卡内全部 ABattery；OutBatteries 不含 nullptr
	void GetAllBatteries(TArray<ABattery*>& OutBatteries) const;

	// 主动注册指定实例，优先级高于 Discover 取到的第一个匹配
	void RegisterMonster(AMonster* InActor);
	void RegisterMineConsole(AMineConsole* InActor);
	void RegisterComputerScreen(AComputerScreenActor* InActor);
	void RegisterTunnel(ATunnel* InActor);
	void RegisterFlashTopLight(AFlashTopLight* InActor);
	void RegisterDoubleAutoDoor(ADoubleAutoDoor* InActor);
	void RegisterFixedCamera(ACameraActor* InActor);

protected:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	void DiscoverActors();

	// mutable：允许在 const Getter 中执行懒发现写入
	mutable TWeakObjectPtr<AMonster> CachedMonster;
	mutable TWeakObjectPtr<AMineConsole> CachedMineConsole;
	mutable TWeakObjectPtr<AComputerScreenActor> CachedComputerScreen;
	mutable TWeakObjectPtr<ATunnel> CachedTunnel;
	mutable TWeakObjectPtr<AFlashTopLight> CachedFlashTopLight;
	mutable TWeakObjectPtr<ADoubleAutoDoor> CachedDoubleAutoDoor;
	mutable TWeakObjectPtr<ACameraActor> CachedFixedCamera;
};

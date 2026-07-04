#include "Input/CrankItInputModeService.h"

#include "PlayerCamera.h"
#include "EnhancedInputSubsystems.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"

// 从 PlayerController 取 World，再获取本子系统实例
UCrankItInputModeService* UCrankItInputModeService::GetFromController(APlayerController* PC)
{
	if (!PC)
	{
		return nullptr;
	}
	UWorld* World = PC->GetWorld();
	return World ? World->GetSubsystem<UCrankItInputModeService>() : nullptr;
}

// 探索阶段默认输入：GameAndUI + 鼠标点击/悬停 + 视口焦点
void UCrankItInputModeService::ApplyExplorationInputMode(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}
	// GameOnly + 可见鼠标时，左键常在 Slate 视口与「世界点击 / EI」之间被反复吞掉。
	// GameAndUI 且不指定 WidgetToFocus：仍把输入交给游戏，同时让鼠标按下能稳定参与 Hit/Click。
	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(Mode);
	PC->bShowMouseCursor = true;
	PC->bEnableClickEvents = true;
	PC->bEnableMouseOverEvents = true;
	FSlateApplication::Get().SetAllUserFocusToGameViewport(EFocusCause::SetDirectly);
}

// 教程/过场期间关闭探索输入（Enhanced Input 映射、鼠标交互、CinematicMode）
void UCrankItInputModeService::DisableAllInput(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}
	// 与 InteractionComponent 联动，暂停 ChargeHandle 等悬停逻辑
	if (APlayerCamera* Cam = Cast<APlayerCamera>(PC->GetPawn()))
	{
		Cam->SetInCinematic(true);
	}
	PC->SetInputMode(FInputModeGameOnly());
	PC->bShowMouseCursor = false;
	SetExplorationMappingContextEnabled(PC, false);
	PC->bEnableClickEvents = false;
	PC->bEnableMouseOverEvents = false;
	PC->SetCinematicMode(true, true, false, true, true);
}

// 教程/过场结束后恢复探索输入
void UCrankItInputModeService::EnableAllInput(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}
	if (APlayerCamera* Cam = Cast<APlayerCamera>(PC->GetPawn()))
	{
		Cam->SetInCinematic(false);
	}
	SetExplorationMappingContextEnabled(PC, true);
	ApplyExplorationInputMode(PC);
	PC->SetCinematicMode(false, false, false, false, false);
}

// 启用/禁用 APlayerCamera::DefaultMappingContext（Enhanced Input 本地子系统）
void UCrankItInputModeService::SetExplorationMappingContextEnabled(APlayerController* PC, bool bEnabled)
{
	if (!PC)
	{
		return;
	}
	APlayerCamera* Cam = Cast<APlayerCamera>(PC->GetPawn());
	if (!Cam || !Cam->DefaultMappingContext)
	{
		return;
	}
	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
	{
		if (bEnabled)
		{
			Subsystem->AddMappingContext(Cam->DefaultMappingContext, 0);
		}
		else
		{
			Subsystem->RemoveMappingContext(Cam->DefaultMappingContext);
		}
	}
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "ComputerScreenActor.h"

#include "PlayerCamera.h"
#include "Camera/CameraActor.h"
#include "ColorManagement/TransferFunctions.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"

// Sets default values
AComputerScreenActor::AComputerScreenActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ScreenMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScreenMesh"));
	SetRootComponent(ScreenMesh);

	ScreenWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ScreenWidget"));
	ScreenWidget->SetupAttachment(ScreenMesh);

	
	ScreenWidget->SetWidgetSpace(EWidgetSpace::World);
	ScreenWidget->SetDrawSize(FVector2D(1920, 1080));
	ScreenWidget->SetPivot(FVector2D(0.5f, 0.5f));

	ScreenMesh->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void AComputerScreenActor::BeginPlay()
{
	Super::BeginPlay();
	TerminalWidget = Cast<UTerminalWidget>(ScreenWidget->GetUserWidgetObject());
	if (TerminalWidget)
	{
		TerminalWidget->AddNewLine(TEXT(">>> Hello World"));
		TerminalWidget->AddNewLine(TEXT(">>> Compiling project..."));
		TerminalWidget->AddNewLine(TEXT(">>> Done!"));
		// 初始状态下禁止获得焦点，只有切到电脑视角时才允许
		TerminalWidget->SetIsFocusable(false);

	}
	FixedCamera = Cast<ACameraActor>(
	UGameplayStatics::GetActorOfClass(GetWorld(), ACameraActor::StaticClass())
);
		
	// 切换摄像机
	PC = GetWorld()->GetFirstPlayerController();
}

void AComputerScreenActor::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);


	if (PC && FixedCamera)
	{
		// 保存原始视角
		APawn* Pawn = PC->GetPawn();
		APlayerCamera* PCamera = Cast<APlayerCamera>(Pawn);
		if(PCamera)
		{
			PCamera->OriginalViewTarget = PC->GetViewTarget();
		}
		
		PC->SetViewTargetWithBlend(FixedCamera, .5f,VTBlend_Cubic); // 平滑切换
	}
}

// Called every frame
void AComputerScreenActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 确保有 PlayerController
	if (!PC && GetWorld())
	{
		PC = GetWorld()->GetFirstPlayerController();
	}

	bool bInComputerView = false;

	if (PC)
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			if (APlayerCamera* PCamera = Cast<APlayerCamera>(Pawn))
			{
				// 当 OriginalViewTarget 非空时，说明当前视角在电脑屏幕相机上
				bInComputerView = (PCamera->OriginalViewTarget != nullptr);
			}
		}
	}

	if (!TerminalWidget && ScreenWidget)
	{
		TerminalWidget = Cast<UTerminalWidget>(ScreenWidget->GetUserWidgetObject());
	}

	// 仅在状态发生变化时更新键盘焦点，避免每帧重复设置
	if (TerminalWidget)
	{
		if (bInComputerView && !bWasInComputerView)
		{
			// 刚刚切换到电脑屏幕视角：允许并设置焦点到终端，玩家可以输入
			// TerminalWidget->bIsFocusable = true;
			TerminalWidget->SetIsFocusable(true);
			TerminalWidget->SetKeyboardFocus();

			if (PC)
			{
				FInputModeGameAndUI InputMode;
				InputMode.SetWidgetToFocus(TerminalWidget->TakeWidget());
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				PC->SetInputMode(InputMode);
				PC->bShowMouseCursor = true;
			}
		}
		else if (!bInComputerView && bWasInComputerView)
		{
			// 刚刚从电脑屏幕视角退出：清除键盘焦点并禁止终端继续接收输入
			FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::SetDirectly);
			// TerminalWidget->bIsFocusable = false;
			TerminalWidget->SetIsFocusable(false);

			if (PC)
			{
				PC->SetInputMode(FInputModeGameOnly());
				PC->bShowMouseCursor = false;
			}

		}
	}

	bWasInComputerView = bInComputerView;
}


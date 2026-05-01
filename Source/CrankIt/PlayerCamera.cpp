// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCamera.h"

#include <rapidjson/document.h>

// #include "ToolBuilderUtil.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SubtitleSubsystem.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "HAL/FileManager.h"
#include "Framework/Application/SlateApplication.h"

// Sets default values
APlayerCamera::APlayerCamera()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SetRootComponent(SpringArmComp);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	BatteryHoldPoint = CreateDefaultSubobject<USceneComponent>(TEXT("BatteryHoldPoint"));
	BatteryHoldPoint->SetupAttachment(CameraComp);

}

void APlayerCamera::ApplyExplorationInputMode(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}
	// GameOnly + 可见鼠标时，左键常在 Slate 视口与「世界点击 / EI」之间被反复吞掉（全程如此，非仅开局）。
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

// Called when the game starts or when spawned
void APlayerCamera::BeginPlay()
{
	Super::BeginPlay();
	PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController)
	{
		ApplyExplorationInputMode(PlayerController);

		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
					ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
			{
				if (DefaultMappingContext)
				{
					Subsystem->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}
	}
	PlayerControllerRef = Cast<APlayerController>(APawn::GetController());

	TArray<AActor*> Found;
	MineConsole = Cast<AMineConsole>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AMineConsole::StaticClass()));

		
	if (PlayerController)
	{
		if (SubtitleWidgetClass)
		{
			SubtitlesWidget = CreateWidget<USubtitleWidget>(PlayerController, SubtitleWidgetClass);
		}
		else
		{
			SubtitlesWidget = CreateWidget<USubtitleWidget>(PlayerController);
		}
		if (SubtitlesWidget)
		{
			SubtitlesWidget->AddToPlayerScreen(200);
		}
	}

	
	// 无语音文件时：用世界时间轴跑几条测试字幕（上面已 AddToPlayerScreen；纯 C++ Widget 会自动建底栏 TextBlock）。
	if (UWorld* World = GetWorld())
	{
		if (USubtitleSubsystem* SubtitleSys = World->GetSubsystem<USubtitleSubsystem>())
		{
			TArray<FCrankItSubtitleCue> TestCues;
			auto AddCue = [&TestCues](float Start, float End, const FString& Msg)
			{
				FCrankItSubtitleCue Cue;
				Cue.StartTimeSeconds = Start;
				Cue.EndTimeSeconds = End;
				Cue.Text = FText::FromString(Msg);
				TestCues.Add(Cue);
			};
			AddCue(0.f, 2.5f, FString(TEXT("【字幕测试】第一句（0~2.5 秒）")));
			AddCue(2.5f, 5.f, FString(TEXT("【字幕测试】第二句（2.5~5 秒）")));
			AddCue(5.f, 8.f, FString(TEXT("【字幕测试】第三句（5~8 秒）")));
			AddCue(8.f, 11.f, FString(TEXT("【字幕测试】第四句（8~11 秒后本轨结束）")));
			SubtitleSys->StartSubtitleTrackWithWorldTime(TestCues);
		}
	}
}

// Called every frame
void APlayerCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FHitResult HitResult;

	if (PlayerControllerRef)
	{
		PlayerControllerRef->GetHitResultUnderCursor(
			ECollisionChannel::ECC_Visibility,
			false,
			HitResult
		);
		DrawDebugSphere(
			GetWorld(),
			HitResult.ImpactPoint,
			25.f,
			12,
			FColor::Red,
			false,
			-1.f
		);

		if (HoldBattery)
		{
			HoldBattery->RootComp->SetWorldLocation(BatteryHoldPoint->GetComponentLocation());
			HoldBattery->RootComp->SetWorldRotation(BatteryHoldPoint->GetComponentRotation());
		}
	}

	//检查鼠标指向对象
	
	UPrimitiveComponent* HitComp = HitResult.GetComponent();
	
	if(HitComp)
	{

		if(HitComp->ComponentHasTag("ChargeHandle"))
		{
			if(MineConsole)
			{
				MineConsole->ShouldRotate = true;
			}	
		}

		else if(HitComp->ComponentHasTag("Battery"))
		{
			if(not HoldBattery)
			{
				TargetBattery = Cast<ABattery>(HitComp->GetOwner());
			}
		}
		else
		{
			if(MineConsole)
			{
				MineConsole->ShouldRotate = false;
				TargetBattery = nullptr;
			}
		}
	}
}



// Called to bind functionality to input
void APlayerCamera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if(EIC) {
		EIC->BindAction(TurnAction, ETriggerEvent::Started, this, &APlayerCamera::TurnInput);
		EIC->BindAction(IntereactAction, ETriggerEvent::Started, this, &APlayerCamera::InteractInput);
		EIC->BindAction(ExitScreen, ETriggerEvent::Started, this, &APlayerCamera::ExitScreenInput);
	}
}


void APlayerCamera::InteractInput(const FInputActionValue& InputActionValue)
{
	if(TargetBattery and not HoldBattery)
	{
		TargetBattery->RootComp->SetWorldLocation(BatteryHoldPoint->GetComponentLocation());
		TargetBattery->RootComp->SetWorldRotation(BatteryHoldPoint->GetComponentRotation());
		HoldBattery = TargetBattery;
		TargetBattery = nullptr;
	}
}

void APlayerCamera::TurnInput(const FInputActionValue& value)
{
	float InputValue = value.Get<float>();
	FRotator TargetRotation = FRotator::ZeroRotator;

	DeltaRotation.Yaw = InputValue * 90.f;
	TargetRotation.Yaw = SpringArmComp->GetComponentRotation().Yaw + DeltaRotation.Yaw;
	
	SpringArmComp->SetWorldRotation(TargetRotation);
}

void APlayerCamera::ExitScreenInput(const FInputActionValue& value)
{
	if (OriginalViewTarget)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();

		PC->SetViewTargetWithBlend(OriginalViewTarget, .5f, VTBlend_Cubic);
		OriginalViewTarget = nullptr; // 清空，避免重复
	}
}


void APlayerCamera::PickBattery()
{
}

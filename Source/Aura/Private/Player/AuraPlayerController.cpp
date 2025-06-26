// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"
#include "Interaction/EnemyInterface.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
}

void AAuraPlayerController::PlayerTick(float Deltatime)
{
	Super::PlayerTick(Deltatime);

	CursorTrace();
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);


	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHit;
    GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);

	if (!CursorHit.bBlockingHit) return;

	PreviousTarget = CurrentTarget;
	CurrentTarget = CursorHit.GetActor();

	/**
	* Line trace from cursor. There are several scenarios :
	*	A. PreviousTarget is null && CurrentTarget is null
	*		- Do nothing.
	*	B. PreviousTarget is null && CurrentTarget is valid
	*		- Highlight CurrentTarget.
	*	C. PreviousTarget is valid && CurrentTarget is null
	*		- Unhighlight PreviousTarget.
	*	D. Both targets are valid but PreviousTarget != CurrentTarget
	*		- Unhighlight PreviousTarget and Highlight CurrentTarget.
	*	E. Both targets are valid and PreviousTarget == CurrentTarget
	*		- Do nothing.
	**/

	if (PreviousTarget == nullptr)
	{
		if (CurrentTarget != nullptr)
		{
			//Case B 
			CurrentTarget->HighlightActor();
		}
		else
		{
			// Case A
			// Do nothing.
		}
	}
	else // Previous Target is valid 
	{
		if (CurrentTarget == nullptr)
		{
			// Case C
			PreviousTarget->UnhighlightActor();
		}
		else if (CurrentTarget != PreviousTarget)
		{
			// Case D
			PreviousTarget->UnhighlightActor();
			CurrentTarget->HighlightActor();
		}
		else
		{
			// Case E
			// Do nothing.
		}

	}
}

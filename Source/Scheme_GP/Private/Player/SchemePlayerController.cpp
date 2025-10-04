// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SchemePlayerController.h"

#include "SchemePlayerPawn.h"
#include "Net/UnrealNetwork.h"
#include "Framework/SchemeGameMode.h"
#include "Interface/InteractableInterface.h"
#include "Kismet/KismetMathLibrary.h"

ASchemePlayerController::ASchemePlayerController()
{
	bReplicates = true;
}

void ASchemePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		if (HasAuthority())
		{
			UE_LOG(LogTemp, Display, TEXT("(CLIENT-SERVER) This player (%s) is both server and client"), *GetName());
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("(CLIENT) This player (%s) is client only"), *GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("(REMOTE) Other player's controller %s"), *GetName());
	}
}

void ASchemePlayerController::SendGoldOutcomeRequestToServer_Implementation(int32 Amount)
{
	if (ASchemeGameMode* GameMode = GetWorld()->GetAuthGameMode<ASchemeGameMode>())
	{
		GameMode->TryProcessGoldOutcome(this, Amount);
	}
}

void ASchemePlayerController::SendGoldIncomeRequestToServer_Implementation(int32 Amount)
{
	if (ASchemeGameMode* GameMode = GetWorld()->GetAuthGameMode<ASchemeGameMode>())
	{
		GameMode->TryProcessGoldIncome(this, Amount);
	}
}

void ASchemePlayerController::ServerRequestInteract_Implementation(AActor* InteractActor, APawn* Interactor)
{
	UE_LOG(LogTemp, Display, TEXT("SERVER: Interact Requested By %s"), *Interactor->GetName());

	IInteractableInterface::Execute_OnInteract(InteractActor, Interactor);
	ClientInteractNotify(InteractActor, Interactor);
}

void ASchemePlayerController::ClientInteractNotify_Implementation(AActor* InteractActor, APawn* Interactor)
{
	IInteractableInterface::Execute_OnInteractionSuccessInClient(InteractActor, Interactor);
}

void ASchemePlayerController::HandleClampedRotation(float MouseInputYaw, float MouseInputPitch)
{
	// If the rotation system disabled then go no further
	if (!bCameraRotationEnabled)
		return;

	if (!CameraRootComp)
	{
		if (ASchemePlayerPawn* PlayerPawn = Cast<ASchemePlayerPawn>(GetPawn()))
		{
			CameraRootComp = PlayerPawn->GetCameraRootComp();
			if (!CameraRootComp)
			{
				UE_LOG(LogTemp, Warning, TEXT("Controller %s: CameraRootComp still NULL, waiting..."), *GetName());
				return;
			}
		}
		else
		{
			return;
		}
	}
	float YawRotation, PitchRotation;
	// Yaw rotation calculation
	float CurrDelta = MouseInputYaw + YawRotationDelta;
	// Check if we reached to yaw limit
	if (MaxYawLimit > UKismetMathLibrary::Abs(CurrDelta))
	{
		// We didn't reach to limit
		YawRotationDelta = CurrDelta;
		// Rotate with MouseInputYaw value
		YawRotation = MouseInputYaw;
	}
	else
	{
		float Delta = UKismetMathLibrary::Abs(CurrDelta) - MaxYawLimit;
		float NewRotationChange = (MouseInputYaw > 0) ? MouseInputYaw - Delta : MouseInputYaw + Delta;

		YawRotationDelta += NewRotationChange;
		// Rotate with NewRotationChange value
		YawRotation = NewRotationChange;
	}
	// Pitch rotation calculation
	CurrDelta = MouseInputPitch + PitchRotationDelta;
	// Check if we reached to pitch limit
	if (MaxPitchLimit > UKismetMathLibrary::Abs(CurrDelta))
	{
		// We didn't reach to limit
		PitchRotationDelta = CurrDelta;
		// Rotate with MouseInputPitch value
		PitchRotation = MouseInputPitch;
	}
	else
	{
		float Delta = UKismetMathLibrary::Abs(CurrDelta) - MaxPitchLimit;
		float NewRotationChange = (MouseInputPitch > 0) ? MouseInputPitch - Delta : MouseInputPitch + Delta;

		PitchRotationDelta += NewRotationChange;
		// Rotate with NewRotationChange value
		PitchRotation = NewRotationChange;
	}
	CameraRootComp->AddLocalRotation(FRotator(PitchRotation, YawRotation, 0));
	FRotator CurrentRotation = CameraRootComp->GetRelativeRotation();
	HandleRotationInServer(CurrentRotation.Yaw, CurrentRotation.Pitch);
}

void ASchemePlayerController::HandleRotationInServer_Implementation(float Yaw, float Pitch)
{
	// Runs in the server
	ASchemePlayerPawn* PlayerPawn = Cast<ASchemePlayerPawn>(GetPawn());
	if (!PlayerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("Controller %s: GetPawn() is NULL!"), *GetName());
		return;
	}

	USceneComponent* CameraRoot = PlayerPawn->GetCameraRootComp();
	if (!CameraRoot)
	{
		UE_LOG(LogTemp, Error, TEXT("Controller %s: CameraRoot is NULL in Pawn %s!"), 
			*GetName(), *PlayerPawn->GetName());
		return;
	}

	// Apply rotation on the server
	FRotator NewRotation = FRotator(Pitch, Yaw, 0);
	CameraRoot->SetRelativeRotation(NewRotation);
	
	// Update the replicated variable (send to other clients)
	PlayerPawn->CameraRotation = NewRotation;
	
	UE_LOG(LogTemp, Display, TEXT("SERVER: Rotation updated for %s to %s"), 
		*PlayerPawn->GetName(), *NewRotation.ToString());
}


// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SchemePlayerController.h"

#include "SchemePlayerPawn.h"
#include "Net/UnrealNetwork.h"
#include "Framework/SchemeGameMode.h"
#include "Framework/SchemeGameState.h"
#include "GameFramework/PlayerState.h"
#include "Interface/InteractableInterface.h"

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

void ASchemePlayerController::SendGameStartRequestToServer_Implementation()
{
	if (ASchemeGameMode* GameMode = GetWorld()->GetAuthGameMode<ASchemeGameMode>())
	{
		GameMode->StartSchemeGame();
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
	UE_LOG(LogTemp, Display, TEXT("Sending"));
	if (ASchemeGameMode* GameMode = GetWorld()->GetAuthGameMode<ASchemeGameMode>())
	{
		UE_LOG(LogTemp, Display, TEXT("CLIENT: Sending Gold Income Request to Server: %d"), Amount);
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

void ASchemePlayerController::SendServerFinishTurnRequest_Implementation()
{
	ASchemeGameMode* GameMode = GetWorld()->GetAuthGameMode<ASchemeGameMode>();
	ASchemeGameState* SchemeGameState = Cast<ASchemeGameState>(GetWorld()->GetGameState());
	if (GameMode && SchemeGameState)
	{
		if (SchemeGameState->CurrentPlayerTurn == PlayerState)
		{
			GameMode->AdvanceTurn();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Turn is not your's to finish!"));
		}
	}
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

	// Calculate what our new delta WOULD be if we applied the full input
	float ProposedYawDelta = CurrentYawDelta + MouseInputYaw;
	float ProposedPitchDelta = CurrentPitchDelta + MouseInputPitch;
	
	// Clamp the proposed deltas to our limits
	float ClampedYawDelta = FMath::Clamp(ProposedYawDelta, -MaxYawLimit, MaxYawLimit);
	float ClampedPitchDelta = FMath::Clamp(ProposedPitchDelta, -MaxPitchLimit, MaxPitchLimit);
	
	// Check if there's actual change
	if (FMath::Abs(ClampedYawDelta - CurrentYawDelta) > KINDA_SMALL_NUMBER || 
		FMath::Abs(ClampedPitchDelta - CurrentPitchDelta) > KINDA_SMALL_NUMBER)
	{
		// Update our tracked deltas
		CurrentYawDelta = ClampedYawDelta;
		CurrentPitchDelta = ClampedPitchDelta;
		
		// This prevents Euler angle order issues (Gimbal Lock)
		FRotator NewRotation = FRotator(CurrentPitchDelta, CurrentYawDelta, 0.f);
		CameraRootComp->SetRelativeRotation(NewRotation);
		
		// Send to server for replication
		HandleRotationInServer(CurrentYawDelta, CurrentPitchDelta);
	}
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
	
	//UE_LOG(LogTemp, Display, TEXT("SERVER: Rotation updated for %s to %s"), 
	//	*PlayerPawn->GetName(), *NewRotation.ToString());
}


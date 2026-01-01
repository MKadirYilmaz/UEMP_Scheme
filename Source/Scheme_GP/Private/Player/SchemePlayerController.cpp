// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SchemePlayerController.h"

#include "Net/UnrealNetwork.h"

#include "SchemePlayerPawn.h"
#include "SchemePlayerState.h"

#include "Framework/SchemeGameMode.h"
#include "Framework/SchemeGameState.h"

#include "Gameplay/Action/SchemeNotification.h"
#include "Gameplay/Data/CardDataAsset.h"
#include "Gameplay/Actors/CardTable.h"
#include "Gameplay/Actors/CardActor.h"

#include "Interface/InteractableInterface.h"
#include "Kismet/GameplayStatics.h"

ASchemePlayerController::ASchemePlayerController()
{
	bReplicates = true;
}

void ASchemePlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	HoldingCards.SetNum(2);
	OnRep_HoldingCards();

	CardTable = Cast<ACardTable>(UGameplayStatics::GetActorOfClass(this, CardTableClass));
	
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

void ASchemePlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ASchemePlayerController, HoldingCards);
}

ASchemeGameMode* ASchemePlayerController::TryGetGameMode() const
{
	return HasAuthority() ? GetWorld()->GetAuthGameMode<ASchemeGameMode>() : nullptr;
		
}

ASchemeGameState* ASchemePlayerController::TryGetGameState() const
{
	return HasAuthority() ? Cast<ASchemeGameState>(GetWorld()->GetGameState()) : nullptr;
}

void ASchemePlayerController::Client_ReceiveNotification_Implementation(const FNotificationPacket& Notification)
{
	if (Notification.NotificationType == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Client %s: Received Notification with NULL Type!"), *GetName());
		return;
	}
	
	USchemeNotification* NotificationInstance = NewObject<USchemeNotification>(this, Notification.NotificationType);
	if (!NotificationInstance) return;
	
	NotificationInstance->HandleNotification(this, Notification.NotificationMessage, 
		Notification.NotificationAsset, Notification.NotificationObjects);
	UE_LOG(LogTemp, Display, TEXT("Client %s: Notification Handled!"), *GetName());
}

void ASchemePlayerController::Server_RequestInteract_Implementation(AActor* InteractActor, APawn* Interactor)
{
	UE_LOG(LogTemp, Display, TEXT("SERVER: Interact Requested By %s"), *Interactor->GetName());

	IInteractableInterface::Execute_OnInteract(InteractActor, Interactor);
	
	// Notify client about successful interaction (for UI updates, etc.)
	Client_InteractNotify(InteractActor, Interactor);
}

void ASchemePlayerController::Client_InteractNotify_Implementation(AActor* InteractActor, APawn* Interactor)
{
	IInteractableInterface::Execute_OnInteractionSuccessInClient(InteractActor, Interactor);
}

void ASchemePlayerController::ExecuteAction_Implementation(UActionDataAsset* ActionData, ASchemePlayerState* TargetState)
{
	if (ASchemeGameMode* GameMode = GetWorld()->GetAuthGameMode<ASchemeGameMode>())
	{
		ASchemePlayerController* Controller = (TargetState) ? Cast<ASchemePlayerController>(TargetState->GetPlayerController()) : nullptr;
		
		GameMode->ProcessActionRequest(this, ActionData, Controller);
	}
}

void ASchemePlayerController::StartGame_Implementation()
{
	if (ASchemeGameMode* GameMode = GetWorld()->GetAuthGameMode<ASchemeGameMode>())
	{
		GameMode->StartSchemeGame();
	}
}

void ASchemePlayerController::Server_SendChallengeRequest_Implementation()
{
	if (ASchemeGameMode* GameMode = GetWorld()->GetAuthGameMode<ASchemeGameMode>())
	{
		GameMode->ProcessChallengeRequest(this);
	}
}

void ASchemePlayerController::Server_SendBlockRequest_Implementation()
{
	if (ASchemeGameMode* GameMode = GetWorld()->GetAuthGameMode<ASchemeGameMode>())
	{
		GameMode->ProcessBlockRequest(this);
	}
}

void ASchemePlayerController::EndTurn_Implementation()
{
	if (ASchemeGameState* SGS = Cast<ASchemeGameState>(GetWorld()->GetGameState()))
	{
		SGS->Server_AdvanceToNextPlayerTurn(this);
	}
}

void ASchemePlayerController::SendChangeGoldRequest_Implementation(int32 Amount)
{
	if (ASchemeGameState* SGS = Cast<ASchemeGameState>(GetWorld()->GetGameState()))
	{
		SGS->Server_ChangePlayerGoldCount(GetPlayerState<ASchemePlayerState>(), Amount);
	}
}

void ASchemePlayerController::Server_AddCardToHand_Implementation(ACardActor* NewCard)
{
	for (int32 i = 0; i < HoldingCards.Num(); i++)
	{
		// Look for an empty slot
		if (HoldingCards[i]) continue;
		
		// Found an empty slot
		HoldingCards[i] = NewCard;
		OnRep_HoldingCards();
		return;
	}
	// No empty slot found
	UE_LOG(LogTemp, Error, TEXT("No empty slot to add the new card in hand!"));
}

void ASchemePlayerController::Server_RemoveCardFromHand_Implementation(UCardDataAsset* CardToRemove)
{
	// Player should put the card in the game mode deck and shuffle it
	for (int32 i = 0; i < HoldingCards.Num(); i++)
	{
		if (!HoldingCards[i]) continue;
		if (HoldingCards[i]->GetCardData()->GetCardRole() == CardToRemove->GetCardRole())
		{
			if (ASchemeGameMode* GameMode = GetWorld()->GetAuthGameMode<ASchemeGameMode>())
			{
				GameMode->ReturnCardToDeck(HoldingCards[i]);
			}
			else return;
			HoldingCards[i] = nullptr;
			OnRep_HoldingCards();
			return;
		}
	}
	UE_LOG(LogTemp, Error, TEXT("The card is not in your hand!"));
}

void ASchemePlayerController::Server_RemoveRandomCardFromHand_Implementation()
{
	// Player should put the card in the game mode deck and shuffle it
	if (HoldingCards.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("No cards in hand to remove!"));
		return;
	}
	// Create a temporary array of indices for real cards (non-null)
	TArray<int32> ValidIndices;
	for (int32 i = 0; i < HoldingCards.Num(); i++)
	{
		if (HoldingCards[i])
		{
			ValidIndices.Add(i);
		}
	}

	if (ValidIndices.Num() == 0) return;

	// Select a random index from the valid indices
	int32 RandomIndex = FMath::RandRange(0, ValidIndices.Num() - 1);
	int32 IndexToRemove = ValidIndices[RandomIndex];

	if (ASchemeGameMode* GameMode = GetWorld()->GetAuthGameMode<ASchemeGameMode>())
	{
		ACardActor* CardToReturn = HoldingCards[IndexToRemove];
		GameMode->ReturnCardToDeck(CardToReturn);
	}
	HoldingCards[IndexToRemove] = nullptr;
	OnRep_HoldingCards();
}

bool ASchemePlayerController::HasCardInHand(const UCardDataAsset* CardToCheck) const
{
	if (!CardToCheck)
	{
		UE_LOG(LogTemp, Error, TEXT("CardToCheck pointer is NULL!"));
		return true;
	}
	for (const ACardActor* HeldCard : HoldingCards)
	{
		if (!HeldCard) continue;
		if (HeldCard->GetCardData()->GetCardRole() == CardToCheck->GetCardRole())
			return true;
	}
	return false;
	
}

bool ASchemePlayerController::HasAnyCardInHand() const
{
	for (const ACardActor* HeldCard : HoldingCards)
	{
		if (HeldCard)
			return true;
	}
	return false;
}

ACardActor* ASchemePlayerController::GetCardFromHand(const ECardRole RoleToGet) const
{
	for (ACardActor* HeldCard : HoldingCards)
	{
		if (!HeldCard) continue;
		if (HeldCard->GetCardData()->GetCardRole() == RoleToGet)
			return HeldCard;
	}
	return nullptr;
}

void ASchemePlayerController::PrintHoldingCards() const
{
	for (const ACardActor* HeldCard : HoldingCards)
	{
		if (!HeldCard) continue;
		UE_LOG(LogTemp, Display, TEXT("Client %s: Holding Card: %s"), *GetName(), *HeldCard->GetCardData()->GetCardName().ToString());
	}
}

void ASchemePlayerController::OnRep_HoldingCards() const
{
	UE_LOG(LogTemp, Display, TEXT("Client %s: HoldingCards Replicated, Num of Cards: %d"),
		*GetName(), HoldingCards.Num());
}

int32 ASchemePlayerController::GetFirstEmptyCardHoldingPointIndex() const
{
	if (!CardTable) return -1;

	for (int32 i = 0; i < HoldingCards.Num(); i++)
	{
		if (!HoldingCards[i])
			return i;
	}
	return -1;
}

FTransform ASchemePlayerController::GetCardHoldingPoint(int32 Index) const
{
	if (!CardTable || Index < 0) return FTransform();

	if (ASchemePlayerState* SchemePlayerState = GetPlayerState<ASchemePlayerState>(); !SchemePlayerState) return FTransform();
	
	int32 PlayerIndex = GetPlayerState<ASchemePlayerState>()->GetPlayerIndex();
	
	if (PlayerIndex >= CardTable->GetCardPointsStructs().Num() || Index >= CardTable->GetCardPointsStructs()[PlayerIndex].CardTransforms.Num())
		return FTransform();
	return CardTable->GetCardPointsStructs()[PlayerIndex].CardTransforms[Index];
}

void ASchemePlayerController::ResetState_Implementation()
{
	for (int32 i = 0; i < HoldingCards.Num(); i++)
	{
		if (HoldingCards[i])
		{
			HoldingCards[i]->Destroy();
			HoldingCards[i] = nullptr;
		}
	}
	OnRep_HoldingCards();
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


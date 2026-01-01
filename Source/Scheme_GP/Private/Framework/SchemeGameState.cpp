// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/SchemeGameState.h"

#include "GameFramework/PlayerState.h"
#include "Player/SchemePlayerController.h"
#include "Player/SchemePlayerState.h"
#include "Net/UnrealNetwork.h"

void ASchemeGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASchemeGameState, CurrentPlayerTurn);
	DOREPLIFETIME(ASchemeGameState, PlayerTurnsOrder);
}

void ASchemeGameState::OnRep_CurrentPlayerTurn()
{
	if (!CurrentPlayerTurn || !CurrentPlayerTurn->GetPawn())
	{
		UE_LOG(LogTemp, Warning, TEXT("Current Player Turn or its Pawn is NULL!"));
		return;
	}
	FVector Start = CurrentPlayerTurn->GetPawn()->GetActorLocation();
	DrawDebugLine(GetWorld(), Start,Start + FVector(0,0,300.f), 
		FColor::Green, false, 5.f, 0, 5.f);
}

void ASchemeGameState::OnRep_PlayerTurnsOrder()
{
	UE_LOG(LogTemp, Display, TEXT("Player Turns Order Changed!"));
}

void ASchemeGameState::StealGoldFromPlayer(ASchemePlayerState* FromPlayer, ASchemePlayerState* ToPlayer, int32 Amount)
{
	if (!HasAuthority() || !FromPlayer || !ToPlayer || Amount <= 0)
		return;
	
	if (FromPlayer->GetGold() < Amount)
	{
		Amount = FromPlayer->GetGold();
	}
	FromPlayer->RemoveGold(Amount);
	ToPlayer->AddGold(Amount);
}

void ASchemeGameState::ResetState_Implementation()
{
	CurrentPlayerTurn = nullptr;
}

void ASchemeGameState::Server_ChangePlayerGoldCount_Implementation(ASchemePlayerState* RequestingPlayerState,
                                                                   int32 Amount)
{
	if (RequestingPlayerState != CurrentPlayerTurn)
	{
		UE_LOG(LogTemp, Error, TEXT("It's not this player's turn to change gold! GameState/ChangeGoldCount"));
		return;
	}
	if (Amount > 0)
	{
		RequestingPlayerState->AddGold(Amount);
	}
	else if (Amount < 0)
	{
		if (!RequestingPlayerState->RemoveGold(FMath::Abs(Amount)))
		{
			UE_LOG(LogTemp, Error, TEXT("Not enough gold to remove! GameState/ChangeGoldCount"));
			// Send notification
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Amount can't be 0! GameState/ChangeGoldCount"));
	}
}

void ASchemeGameState::Server_AdvanceToNextPlayerTurn_Implementation(ASchemePlayerController* RequestingController)
{
	if (!RequestingController) return;
	APlayerState* RequestingPlayerState = RequestingController->GetPlayerState<APlayerState>();
	if (!RequestingPlayerState) return;
	
	if (CurrentPlayerTurn != RequestingPlayerState)
	{
		UE_LOG(LogTemp, Display, TEXT("Not this player's turn to advance!"));
		return;
	}
	ForceAdvanceTurn();
}

void ASchemeGameState::ForceAdvanceTurn()
{
	if (PlayerTurnsOrder.Num() == 0) return;

	int32 CurrentIndex = PlayerTurnsOrder.IndexOfByKey(CurrentPlayerTurn);
	// If CurrentPlayerTurn is not in the list (e.g. just removed), CurrentIndex is -1.
	// In that case, we start from -1, so NextIndex becomes 0.
	
	int32 NextIndex;
	int32 FreezeProtectionCounter = 0;
	
	do
	{
		FreezeProtectionCounter++;
		NextIndex = (CurrentIndex + 1) % PlayerTurnsOrder.Num();
		CurrentIndex = NextIndex; // Advance for next iteration check
		
		// Check if we have looped through all players
		if (FreezeProtectionCounter > PlayerTurnsOrder.Num() + 1)
		{
			UE_LOG(LogTemp, Error, TEXT("All players are eliminated or list empty! No valid next player turn."));
			return;
		}
	} while (PlayerTurnsOrder[NextIndex] && Cast<ASchemePlayerState>(PlayerTurnsOrder[NextIndex])->IsEliminated());
	
	CurrentPlayerTurn = PlayerTurnsOrder[NextIndex];
	OnRep_CurrentPlayerTurn();
}

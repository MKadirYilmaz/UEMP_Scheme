// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/SchemeGameState.h"

#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

void ASchemeGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASchemeGameState, CurrentPlayerTurn);
	DOREPLIFETIME(ASchemeGameState, PlayerTurnsOrder);
}

void ASchemeGameState::OnRep_CurrentPlayerTurn()
{
	/*
	UE_LOG(LogTemp, Warning, TEXT(" (%s): Current Player Turn Changed! New Player In Turn: %s"),
		(HasAuthority()) ? TEXT("Server") : TEXT("Client"),
		*CurrentPlayerTurn->GetName());
	*/
}

void ASchemeGameState::OnRep_PlayerTurnsOrder()
{
	UE_LOG(LogTemp, Display, TEXT("Player Turns Order Changed!"));
}

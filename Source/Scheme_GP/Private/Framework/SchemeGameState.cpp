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

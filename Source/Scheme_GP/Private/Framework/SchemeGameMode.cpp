// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/SchemeGameMode.h"

#include "GameFramework/GameState.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/SchemePlayerState.h"


bool ASchemeGameMode::TryProcessGoldIncome(APlayerController* RequestingController, int32 Amount)
{
	if (!RequestingController) return false;

	ASchemePlayerState* PlayerState = RequestingController->GetPlayerState<ASchemePlayerState>();
	if (!PlayerState) return false;

	PlayerState->AddGold(Amount);
	return true;
}

bool ASchemeGameMode::TryProcessGoldOutcome(APlayerController* RequestingController, int32 Amount)
{
	if (!RequestingController) return false;

	ASchemePlayerState* PlayerState = RequestingController->GetPlayerState<ASchemePlayerState>();
	if (!PlayerState) return false;
	
	return PlayerState->RemoveGold(Amount);
}

void ASchemeGameMode::CreateVirtualDeck(int32 NumOfPlayers)
{
	VirtualGameDeck.Empty();
	
	if (!GameState) return;
	NumOfPlayers = GameState->PlayerArray.Num();

	int32 CopiesPerCard = 3;
	if (NumOfPlayers > 7) CopiesPerCard = 4;

	for (UCardDataAsset* Card : AllCardDataTypes)
	{
		for (int32 i = 0; i < CopiesPerCard; i++)
		{
			VirtualGameDeck.Add(Card);
		}
	}
}

void ASchemeGameMode::ShuffleDeck()
{
	TArray<UCardDataAsset*> TempDeck;
	while (VirtualGameDeck.Num() > 0)
	{
		int32 RandIndex = UKismetMathLibrary::RandomInteger(VirtualGameDeck.Num());
		TempDeck.Add(VirtualGameDeck[RandIndex]);
		VirtualGameDeck.RemoveAt(RandIndex);
	}
	VirtualGameDeck = TempDeck;
}

void ASchemeGameMode::DrawCard(APawn* DrawingPawn)
{
	if (!DrawingPawn) return;
	if (VirtualGameDeck.Num() == 0) return;
	UCardDataAsset* Card = VirtualGameDeck.Pop();
	
}

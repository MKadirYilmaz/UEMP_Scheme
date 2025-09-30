// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SchemePlayerState.h"

#include "Gameplay/Data/CardDataAsset.h"

void ASchemePlayerState::AddGold(int32 amount)
{
	if (amount < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Negative gold amount is not allowed"));
		return;
	}
	Gold += amount;
	OnGoldChanged.Broadcast(this, Gold, amount);
}

bool ASchemePlayerState::RemoveGold(int32 amount)
{
	if (amount < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Negative gold amount is not allowed"));
		return false;
	}
	if (Gold < amount) return false;
	
	Gold -= amount;
	OnGoldChanged.Broadcast(this, Gold, -amount);
	
	return true;
}

void ASchemePlayerState::AddCardToHand(UCardDataAsset* NewCard)
{
	HoldingCards.Add(NewCard);
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SchemePlayerState.h"

#include "Gameplay/Data/CardDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Actors/CardTable.h"
#include "Net/UnrealNetwork.h"

void ASchemePlayerState::BeginPlay()
{
	Super::BeginPlay();
	if (AActor* TableActor = UGameplayStatics::GetActorOfClass(this, CardTableClass))
	{
		CardTable = Cast<ACardTable>(TableActor);
	}
}

ASchemePlayerState::ASchemePlayerState()
{
	bReplicates = true;
}

void ASchemePlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASchemePlayerState, Gold);
	DOREPLIFETIME(ASchemePlayerState, HoldingCards);
}

void ASchemePlayerState::AddGold(int32 amount)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("Only the server can add gold"));
		return;
	}
	if (amount < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Negative gold amount is not allowed"));
		return;
	}
	CachedDelta = amount;
	Gold += amount;
	UE_LOG(LogTemp, Warning, TEXT("SERVER: New Gold: %d"), Gold);

	// Notify blueprint on server
	OnGoldChange(Gold, amount);
}

bool ASchemePlayerState::RemoveGold(int32 amount)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("Only the server can remove gold"));
		return false;
	}
	if (amount < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Negative gold amount is not allowed"));
		return false;
	}
	if (Gold < amount)
	{
		UE_LOG(LogTemp, Error, TEXT("SERVER: Not enough gold to remove"));
		return false;
	}
	CachedDelta = -amount;
	Gold -= amount;
	UE_LOG(LogTemp, Display, TEXT("SERVER: New Gold: %d"), Gold);

	OnGoldChange(Gold, -amount);
	return true;
}

void ASchemePlayerState::AddCardToHand(UCardDataAsset* NewCard)
{
	HoldingCards.Add(NewCard);
}

FTransform ASchemePlayerState::GetNextCardHoldingPoint()
{
	if (!CardTable)
		return FTransform();
	if (PlayerIndex >= CardTable->GetCardPointsStructs().Num() || CardHoldingPointIndex >= CardTable->GetCardPointsStructs()[PlayerIndex].CardTransforms.Num())
		return FTransform();
	FTransform& Transform = CardTable->GetCardPointsStructs()[PlayerIndex].CardTransforms[CardHoldingPointIndex];
	CardHoldingPointIndex++;
	return Transform;
}

void ASchemePlayerState::OnRep_Gold()
{
	// Called automatically in clients when the gold value has changed.

	OnGoldChange(Gold, CachedDelta);
}

void ASchemePlayerState::OnRep_HoldingCards()
{
	// Called automatically in clients when the holding cards have changed.

	OnCardChange(HoldingCards);
	UE_LOG(LogTemp, Display, TEXT("Holding Cards Have Changed In: %s"), *GetName());
}

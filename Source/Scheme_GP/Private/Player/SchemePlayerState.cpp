// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SchemePlayerState.h"

#include "Net/UnrealNetwork.h"

void ASchemePlayerState::BeginPlay()
{
	Super::BeginPlay();
}

ASchemePlayerState::ASchemePlayerState()
{
	bReplicates = true;
}

void ASchemePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASchemePlayerState, Gold);
	DOREPLIFETIME(ASchemePlayerState, PlayerIndex);
	DOREPLIFETIME(ASchemePlayerState, Username);
	DOREPLIFETIME(ASchemePlayerState, bIsEliminated);
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

void ASchemePlayerState::ResetState_Implementation()
{
	Gold = 2;
	bIsEliminated = false;
}

void ASchemePlayerState::OnRep_Gold()
{
	// Called automatically in clients when the gold value has changed.

	OnGoldChange(Gold, CachedDelta);
}

void ASchemePlayerState::OnRep_Username()
{
	UE_LOG(LogTemp, Display, TEXT("Player Name Changed: %s"), *Username.ToString());
}

void ASchemePlayerState::OnRep_PlayerIndex()
{
	UE_LOG(LogTemp, Display, TEXT("Player Index Changed: %s"), *Username.ToString());
}

void ASchemePlayerState::OnRep_IsEliminated()
{
	UE_LOG(LogTemp, Display, TEXT("Player Elimination Status Changed: %s"), *Username.ToString());
}

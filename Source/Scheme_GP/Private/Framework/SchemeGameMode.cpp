// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/SchemeGameMode.h"

#include "GameFramework/GameState.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/SchemePlayerState.h"


void ASchemeGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Display, TEXT("This is GameMode! Authority Check: %s"),
		HasAuthority() ? TEXT("TRUE (Server)") : TEXT("FALSE"));
}

void ASchemeGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer)
		return;

	UE_LOG(LogTemp, Display, TEXT("New Player Has Connected! %s"), *NewPlayer->GetName());
	if (ASchemePlayerState* PlayerState = NewPlayer->GetPlayerState<ASchemePlayerState>())
	{
		UE_LOG(LogTemp, Display, TEXT("		Player State Created! %s"), *PlayerState->GetName());
	}
}

void ASchemeGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (!Exiting)
		return;

	UE_LOG(LogTemp, Display, TEXT("Player Has Left! %s"), *Exiting->GetName());
}

void ASchemeGameMode::TryProcessGoldIncome_Implementation(APlayerController* RequestingController, int32 Amount)
{
	if (!RequestingController) return;

	ASchemePlayerState* PlayerState = RequestingController->GetPlayerState<ASchemePlayerState>();
	if (!PlayerState) return;

	PlayerState->AddGold(Amount);
}

void ASchemeGameMode::TryProcessGoldOutcome_Implementation(APlayerController* RequestingController, int32 Amount)
{
	if (!RequestingController) return;

	ASchemePlayerState* PlayerState = RequestingController->GetPlayerState<ASchemePlayerState>();
	if (!PlayerState) return ;
	
	PlayerState->RemoveGold(Amount);
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

void ASchemeGameMode::DealInitialCards(int32 CardsPerPlayer)
{
	TArray<TObjectPtr<APlayerState>> PlayerStates = GameState->PlayerArray;
	for (int32 i = 0; i < CardsPerPlayer; i++)
	{
		for (TObjectPtr<APlayerState> PlayerState : PlayerStates)
		{
			if (ASchemePlayerState* SchemePlayerState = Cast<ASchemePlayerState>(PlayerState))
			{
				DrawCard(SchemePlayerState);
			}
		}
	}
}

void ASchemeGameMode::DrawCard(ASchemePlayerState* PlayerState)
{
	if (!PlayerState) return;
	if (VirtualGameDeck.Num() == 0) return;
	UCardDataAsset* Card = VirtualGameDeck.Pop();

	PlayerState->AddCardToHand(Card);
}

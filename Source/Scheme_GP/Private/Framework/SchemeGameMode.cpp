// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/SchemeGameMode.h"

#include "GameFramework/GameState.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/SchemePlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Gameplay/Actors/CardActor.h"


void ASchemeGameMode::BeginPlay()
{
	Super::BeginPlay();
	FindAllStartLocations();
	UE_LOG(LogTemp, Display, TEXT("This is GameMode! Authority Check: %s"),
		HasAuthority() ? TEXT("TRUE (Server)") : TEXT("FALSE"));
}

void ASchemeGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer)
		return;

	UE_LOG(LogTemp, Warning, TEXT("New Player Has Connected! %s"), *NewPlayer->GetName());
	if (ASchemePlayerState* PlayerState = NewPlayer->GetPlayerState<ASchemePlayerState>())
	{
		UE_LOG(LogTemp, Warning, TEXT("		Player State Created! %s"), *PlayerState->GetName());
		PlayerState->SetPlayerIndex(CurrentPlayers.Num());
		
		CurrentPlayers.Add(NewPlayer);
		
		if (CurrentPlayers.Num() > MinPlayer && CurrentPlayers.Num() < MaxPlayer)
		{
			bCanGameStart = true;
			OnCanGameStartValid();
		}
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

AActor* ASchemeGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (PlayerStartLocations.IsEmpty())
		FindAllStartLocations();
	if (PlayerStartLocations.Num() > GetNumPlayers() && PlayerStartLocations[GetNumPlayers() - 1])
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawning Player: %s, Current Player Number: %d"), *Player->GetName(), GetNumPlayers());
		APlayerStart* PlayerStart = PlayerStartLocations[GetNumPlayers() - 1];
		return PlayerStart;
	}
	UE_LOG(LogTemp, Error, TEXT("Player Start Couldn't Found!"));
	return Super::ChoosePlayerStart_Implementation(Player);
}

void ASchemeGameMode::CreateVirtualDeck()
{
	VirtualGameDeck.Empty();
	
	if (!GameState) return;
	int32 NumOfPlayers = CurrentPlayers.Num();

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
	TArray<APlayerState*> PlayerStates {};
	for (APlayerController* PlayerController : CurrentPlayers)
	{
		PlayerStates.Add(PlayerController->PlayerState);
	}
	UE_LOG(LogTemp, Display, TEXT("%s: Num of Player In The Game: %d"), (HasAuthority()) ? TEXT("SERVER") : TEXT("CLIENT"), GetNumPlayers());
	for (int32 i = 0; i < CardsPerPlayer; i++)
	{
		for (APlayerState* PlayerState : PlayerStates)
		{
			ASchemePlayerState* SchemePlayerState = Cast<ASchemePlayerState>(PlayerState);
			if (!SchemePlayerState) continue;
				DrawCard(SchemePlayerState);
		}
	}
}

void ASchemeGameMode::DrawCard(ASchemePlayerState* PlayerState)
{
	if (!PlayerState) return;
	if (VirtualGameDeck.Num() == 0) return;
	UCardDataAsset* Card = VirtualGameDeck.Pop();

	// Add Virtual Card
	PlayerState->AddCardToHand(Card);
	// Spawn Visual Actor
	FTransform SpawnTransform = PlayerState->GetNextCardHoldingPoint();
	ACardActor* SpawnedCard = GetWorld()->SpawnActor<ACardActor>(CardActorClass, SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator());
	SpawnedCard->SetCardData(Card);
}

void ASchemeGameMode::StartSchemeGame()
{
	if (!bCanGameStart)
	{
		UE_LOG(LogTemp, Error, TEXT("You can't start the game yet!"));
		return;
	}
	if (bIsGameStarted)
	{
		UE_LOG(LogTemp, Error, TEXT("The game has already started!"));
		return;
	}
	CreateVirtualDeck();
	ShuffleDeck();
	DealInitialCards(CardAmountPerPlayer);
	bIsGameStarted = true;
}

void ASchemeGameMode::FindAllStartLocations()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), FoundActors);

	PlayerStartLocations.SetNum(8);
	
	for (AActor* FoundActor : FoundActors)
	{
		APlayerStart* PlayerStart = Cast<APlayerStart>(FoundActor);
		if (!PlayerStart) continue;

		for (int32 i = 0; i < 8; i++)
		{
			if (PlayerStart->PlayerStartTag.IsEqual(*FString::Printf(TEXT("Player%d"), i)))
				PlayerStartLocations[i] = PlayerStart;
		}
	}
}

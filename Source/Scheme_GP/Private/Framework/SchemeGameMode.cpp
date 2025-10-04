// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/SchemeGameMode.h"

#include "GameFramework/GameState.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/SchemePlayerState.h"
#include "GameFramework/PlayerStart.h"


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

	UE_LOG(LogTemp, Warning, TEXT("New Player Has Connected! %s"), *NewPlayer->GetName());
	if (ASchemePlayerState* PlayerState = NewPlayer->GetPlayerState<ASchemePlayerState>())
	{
		UE_LOG(LogTemp, Warning, TEXT("		Player State Created! %s"), *PlayerState->GetName());
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
	if (PlayerStartLocations.Num() > 0)
	{
		if (GetNumPlayers() < PlayerStartLocations.Num())
			return PlayerStartLocations[GetNumPlayers()];
	}
	
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

	PlayerState->AddCardToHand(Card);
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
	for (AActor* Actor : FoundActors)
	{
		if (APlayerStart* PlayerStart = Cast<APlayerStart>(Actor))
		{
			PlayerStartLocations.Add(PlayerStart);
		}
	}
}

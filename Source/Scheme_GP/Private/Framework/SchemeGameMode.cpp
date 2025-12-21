// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/SchemeGameMode.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/PlayerStart.h"

#include "SchemeGameState.h"
#include "Player/SchemePlayerState.h"
#include "Player/SchemePlayerController.h"

#include "Gameplay/Actors/CardActor.h"
#include "Gameplay/Action/BaseAction.h"
#include "Gameplay/Data/ActionDataAsset.h"
#include "Gameplay/Data/CardDataAsset.h"


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
		if (ASchemeGameState* SGS = Cast<ASchemeGameState>(GameState))
		{
			SGS->PlayerTurnsOrder.Add(PlayerState);
		}
		
		if (CurrentPlayers.Num() >= MinPlayer && CurrentPlayers.Num() <= MaxPlayer)
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

void ASchemeGameMode::ProcessPlayerAction(ASchemePlayerController* RequestingController, UActionDataAsset* ActionData, ASchemePlayerController* TargetController)
{
	// Process any server-side logic for the player's action here
	if (!ActionData || !ActionData->ActionLogicClass || !RequestingController) return;
	
	if (RequestingController->GetPlayerState<ASchemePlayerState>() != GetGameState<ASchemeGameState>()->CurrentPlayerTurn)
	{
		SendGeneralNotificationToPlayer(RequestingController, FText::FromString("It's not your turn to perform an action."));
		return;
	}
	
	UBaseAction* ActionInstance = NewObject<UBaseAction>(this, ActionData->ActionLogicClass);
	
	if (!ActionInstance) return;
	
	// Notify all players about the action being performed
	if (ActionData->bCanBeInterrupted)
	{
		
		CurrChallengeAction = ActionData;
		CurrActionController = RequestingController;
		CurrTargetController = TargetController;
		
		FNotificationPacket Packet = {
			ServerNotificationMap.Find(EServerNotificationType::ChallengeNotification)->Get(),
			FText(),
			ActionData
		};
		BroadcastPlayerActionNotification(RequestingController, TargetController, Packet);
		// Extra protection against multiple timers
		if (ChallengeTimeoutHandle.IsValid())
		{
			GetWorldTimerManager().ClearTimer(ChallengeTimeoutHandle);
		}
		// Last timer is now unusable, recreate it
		ChallengeTimeoutHandle = FTimerHandle{};
		// Wait for challenge responses before executing the action
		GetWorld()->GetTimerManager().SetTimer(ChallengeTimeoutHandle, this, &ASchemeGameMode::BroadcastTimeoutNotification, 10.0f, false);
		
	}
	// Execute the action immediately
	else
	{
		FNotificationPacket Packet = {
			ServerNotificationMap.Find(EServerNotificationType::GeneralNotification)->Get(),
			FText::FromString("An action has been executed."),
			ActionData
		};

		BroadcastNotificationPacket(Packet);
		ActionInstance->ExecuteAction(RequestingController, TargetController);
	}
}

void ASchemeGameMode::BroadcastPlayerActionNotification(ASchemePlayerController* ActionDealer, ASchemePlayerController* ActionTarget, FNotificationPacket& Packet)
{
	for (APlayerController* PlayerController : CurrentPlayers)
	{
		ASchemePlayerController* SchemePlayerController = Cast<ASchemePlayerController>(PlayerController);
		if (!SchemePlayerController || SchemePlayerController == ActionDealer) continue;
		
		if (SchemePlayerController == ActionTarget)
		{
			Packet.NotificationMessage = FText::FromString("You are being targeted by an action!");
			SchemePlayerController->Client_ReceiveNotification(Packet);
		}
		else
		{
			Packet.NotificationMessage = FText::FromString("Another player is being targeted by an action!");
			SchemePlayerController->Client_ReceiveNotification(Packet);
		}
	}
}

void ASchemeGameMode::BroadcastTimeoutNotification()
{
	for (APlayerController* PlayerController : CurrentPlayers)
	{
		FNotificationPacket Packet = {
			ServerNotificationMap.Find(EServerNotificationType::TimeoutNotification)->Get(),
			FText::FromString("The action challenge period has timed out.")
		};
		ASchemePlayerController* SchemePlayerController = Cast<ASchemePlayerController>(PlayerController);
		if (!SchemePlayerController) continue;
		
		SchemePlayerController->Client_ReceiveNotification(Packet);
	}
	
	// Execute the action since the challenge period has ended
	UBaseAction* ActionInstance = NewObject<UBaseAction>(this, CurrChallengeAction->ActionLogicClass);
	if (!ActionInstance) return;
	ActionInstance->ExecuteAction(CurrActionController, CurrTargetController);
}

void ASchemeGameMode::BroadcastNotificationPacket(const FNotificationPacket& Packet)
{
	for (APlayerController* PlayerController : CurrentPlayers)
	{
		ASchemePlayerController* SchemePlayerController = Cast<ASchemePlayerController>(PlayerController);
		if (!SchemePlayerController) continue;
		
		SchemePlayerController->Client_ReceiveNotification(Packet);
	}
}

void ASchemeGameMode::SendGeneralNotificationToPlayer(ASchemePlayerController* TargetPlayer, const FText& Message)
{
	FNotificationPacket Packet = {
		ServerNotificationMap.Find(EServerNotificationType::GeneralNotification)->Get(),
		Message
	};
	TargetPlayer->Client_ReceiveNotification(Packet);
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

void ASchemeGameMode::BreakTimeoutCountdown()
{
	GetWorldTimerManager().ClearTimer(ChallengeTimeoutHandle);
	
	FNotificationPacket Packet = {
		ServerNotificationMap.Find(EServerNotificationType::TimeoutNotification)->Get(),
		FText::FromString("Someone challenged the action!"),
	};
	BroadcastNotificationPacket(Packet);
}

void ASchemeGameMode::ProcessChallengeRequest()
{
	if (!CurrChallengeAction || !CurrActionController)
	{
		UE_LOG(LogTemp, Error, TEXT("No current challenge action or action controller set!"));
		return;
	}
	BreakTimeoutCountdown();
	// Determine who won the challenge (for simplicity, we'll assume the action always succeeds here)
	FNotificationPacket Packet = {
		ServerNotificationMap.Find(EServerNotificationType::GeneralNotification)->Get(),
		FText(),
		CurrChallengeAction
	};
	
	bool ChallengeResult = CurrActionController->HasCardInHand(CurrChallengeAction->RequiredCardToPerform);
	Packet.NotificationMessage = (ChallengeResult) ? FText::FromString(" The action succeeds as the required card is present. Challenge failed.")
		: FText::FromString(" The action fails as the required card is absent. Challenge succeeded.");
	
	BroadcastNotificationPacket(Packet);
	
	if (ChallengeResult)
	{
		UBaseAction* ActionInstance = NewObject<UBaseAction>(this, CurrChallengeAction->ActionLogicClass);
		if (!ActionInstance) return;
		ActionInstance->ExecuteAction(CurrActionController, CurrTargetController);
		
		// Since the all player know the card exist in the action dealer's hand, remove it
		CurrActionController->Server_RemoveCardFromHand(CurrChallengeAction->RequiredCardToPerform);
		// Then draw a new card to replace it
		DrawCard(CurrActionController, CurrActionController->GetFirstEmptyCardHoldingPointIndex());
		
		// Punish the challenger
	}
	else
	{
		if (CurrActionController->HasAnyCardInHand())
		{
			CurrActionController->Server_RemoveRandomCardFromHand();
		}
		else
		{
			SendGeneralNotificationToPlayer(CurrActionController, FText::FromString("You have no cards left to remove!"));
			// Action dealer lose the game
		}
	}
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
		
		ASchemePlayerController* SchemePlayerController = Cast<ASchemePlayerController>(PlayerController);
		if (!SchemePlayerController) continue;
		
		for (int32 i = 0; i < CardsPerPlayer; i++)
		{
			DrawCard(SchemePlayerController, SchemePlayerController->GetFirstEmptyCardHoldingPointIndex());
		}
		
	}
	UE_LOG(LogTemp, Display, TEXT("%s: Num of Player In The Game: %d"), (HasAuthority()) ? TEXT("SERVER") : TEXT("CLIENT"), GetNumPlayers());
	
}

void ASchemeGameMode::DrawCard(ASchemePlayerController* PlayerController, int32 HoldIndex)
{
	if (!PlayerController || HoldIndex == -1 || VirtualGameDeck.Num() == 0) return;
	
	ASchemePlayerState* PlayerState = PlayerController->GetPlayerState<ASchemePlayerState>();
	if (!PlayerState) return;

	UCardDataAsset* Card = VirtualGameDeck.Pop();
	// Spawn Visual Actor
	FTransform SpawnTransform = PlayerController->GetCardHoldingPoint(PlayerController->GetFirstEmptyCardHoldingPointIndex());
	
	ACardActor* SpawnedCard = GetWorld()->SpawnActor<ACardActor>(CardActorClass, SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator());
	SpawnedCard->SetCardData(Card);
	
	// Add Virtual Card
	PlayerController->Server_AddCardToHand(SpawnedCard);
	
}

void ASchemeGameMode::ReturnCardToDeck(AActor* CardToReturn)
{
	if (!CardToReturn) return;
	ACardActor* CardActor = Cast<ACardActor>(CardToReturn);
	if (!CardActor) return;
	
	UCardDataAsset* CardData = CardActor->GetCardData();
	if (!CardData) return;
	
	VirtualGameDeck.Add(CardData);
	
	ShuffleDeck();
	
	CardToReturn->Destroy();
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
	if (ASchemeGameState* SchemeGameState = GetWorld()->GetGameState<ASchemeGameState>())
	{
		SchemeGameState->CurrentPlayerTurn = CurrentPlayers[CurrentTurnIndex]->PlayerState;
		SchemeGameState->OnRep_CurrentPlayerTurn();
	}
	bIsGameStarted = true;
	
	for (APlayerController* PlayerController : CurrentPlayers)
	{
		if (ASchemePlayerController* SchemePC = Cast<ASchemePlayerController>(PlayerController))
		{
			TSubclassOf<USchemeNotification> NotifyClass = ServerNotificationMap.Find(EServerNotificationType::GameStartNotification)->Get();
			SchemePC->Client_ReceiveNotification(FNotificationPacket{NotifyClass, FText::FromString("The Game Has Started!")});
		}
	}
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

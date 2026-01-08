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
		PlayerState->SetUsername(FText::FromString(FString::Printf(TEXT("Player%d"), PlayerState->GetPlayerIndex())));
		
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

	ASchemePlayerController* ExitingPC = Cast<ASchemePlayerController>(Exiting);
	
	// 1. Handle Active Context / Phase
	if (bIsGameStarted && ExitingPC)
	{
		bool bResetPhase = false;
		if (CurrentContext.InstigatorCont == ExitingPC)
		{
			// Instigator left. Cancel action.
			SendGeneralNotificationToPlayer(CurrentContext.Target, FText::FromString("Action cancelled because the instigator left the game."));
			bResetPhase = true;
		}
		else if (CurrentContext.Target == ExitingPC)
		{
			// Target left.
			SendGeneralNotificationToPlayer(CurrentContext.InstigatorCont, FText::FromString("Action cancelled because the target left the game."));
			bResetPhase = true;
		}
		else if (CurrentContext.Challenger == ExitingPC)
		{
			// Challenger left.
			bResetPhase = true;
		}
		else if (CurrentContext.Blocker == ExitingPC)
		{
			// Blocker left.
			bResetPhase = true;
		}

		if (bResetPhase)
		{
			CurrentContext.Reset();
			SetGamePhase(EGamePhase::Idle);
			// Clear any timers
			GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
		}
	}

	// 2. Remove from lists first
	CurrentPlayers.Remove(Cast<APlayerController>(Exiting));
	
	bool bWasCurrentTurn = false;
	ASchemeGameState* SGS = Cast<ASchemeGameState>(GameState);
	
	if (SGS)
	{
		if (ASchemePlayerState* SPS = Exiting->GetPlayerState<ASchemePlayerState>())
		{
			bWasCurrentTurn = (SGS->CurrentPlayerTurn == SPS);
			SGS->PlayerTurnsOrder.Remove(SPS);
		}
	}

	// 3. Check Win Condition (This might end the game)
	if (bIsGameStarted)
	{
		CheckWinCondition();
	}
	
	// 4. Handle Turn System
	// Only advance turn if the game is STILL started and, it was the exiting player's turn.
	if (bIsGameStarted && bWasCurrentTurn && SGS)
	{
		// Force advance turn since the current player left
		SGS->ForceAdvanceTurn();
	}
}

void ASchemeGameMode::BroadcastNotificationPacket(const FNotificationPacket& Packet, const ASchemePlayerController* ExcludePlayer)
{
	for (APlayerController* PlayerController : CurrentPlayers)
	{
		ASchemePlayerController* SchemePlayerController = Cast<ASchemePlayerController>(PlayerController);
		if (!SchemePlayerController || SchemePlayerController == ExcludePlayer) continue;
		
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
	UE_LOG(LogTemp, Error, TEXT("Player Start Could Not Find"));
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
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = PlayerController;
	
	
	// Spawn Visual Actor
	FTransform SpawnTransform = PlayerController->GetCardHoldingPoint(PlayerController->GetFirstEmptyCardHoldingPointIndex());
	
	ACardActor* SpawnedCard = GetWorld()->SpawnActor<ACardActor>(CardActorClass, SpawnTransform.GetLocation(), 
		SpawnTransform.GetRotation().Rotator(), SpawnParams);
	
	if (!SpawnedCard) return;
	
	SpawnedCard->SetCardData(Card);
	SpawnedCard->OnRep_CardData();
	
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
		
		Cast<ASchemePlayerController>(SchemeGameState->CurrentPlayerTurn->GetPlayerController())->Client_OnTurnComesToPlayer();
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


void ASchemeGameMode::ProcessActionRequest(ASchemePlayerController* InstigatorController, UActionDataAsset* ActionData,
	ASchemePlayerController* Target)
{
	if (CurrentPhase != EGamePhase::Idle)
	{
		SendGeneralNotificationToPlayer(InstigatorController, FText::FromString("Cannot perform action right now."));
		return;
	}

	// Check Forced Action Rule
	if (ForcedActionData)
	{
		if (ASchemePlayerState* PS = InstigatorController->GetPlayerState<ASchemePlayerState>())
		{
			if (PS->ShouldForceAct())
			{
				if (ActionData != ForcedActionData)
				{
					SendGeneralNotificationToPlayer(InstigatorController, FText::FromString("You have too much gold! You are forced to use the special action."));
					return;
				}
			}
		}
	}
	
	CurrentContext.Reset();
	CurrentContext.InstigatorCont = InstigatorController;
	CurrentContext.Target = Target;
	CurrentContext.ActionData = ActionData;
	
	if (!ActionData->RequiredCardToPerform && !ActionData->BlockableByCard)
	{
		// No challenge or block possible, execute immediately
		ExecuteCurrentAction();
	}
	else
	{
		SetGamePhase(EGamePhase::ActionReaction);
	}
}

void ASchemeGameMode::ProcessChallengeRequest(ASchemePlayerController* Challenger)
{
	// Only allow challenges during reaction phases
	if (CurrentPhase != EGamePhase::ActionReaction && CurrentPhase != EGamePhase::BlockReaction) return;
	
	// Prevent self-challenge
	if (CurrentPhase == EGamePhase::ActionReaction && Challenger == CurrentContext.InstigatorCont) return; // This will allow block challenge but block action challenge to self
	
	CurrentContext.Challenger = Challenger;
	
	// Stop the phase timer
	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	
	if (CurrentPhase == EGamePhase::ActionReaction)
	{
		BroadcastNotificationPacket(
			FNotificationPacket{ServerNotificationMap.Find(EServerNotificationType::TimeoutNotification)->Get(),
			FText::FromString("An action challenge has been declared.")}
		);
		
		AutoResolveChallenge(CurrentContext.InstigatorCont, Challenger, CurrentContext.ActionData->RequiredCardToPerform);
		return;
	}
	if (CurrentPhase == EGamePhase::BlockReaction)
	{
		BroadcastNotificationPacket(
			FNotificationPacket{ServerNotificationMap.Find(EServerNotificationType::TimeoutNotification)->Get(),
			FText::FromString("A block challenge has been declared.")}
		);
		
		AutoResolveChallenge(CurrentContext.Blocker, Challenger, CurrentContext.ActionData->BlockableByCard);
		return;
	}
	// Move to processing phase
	CurrentPhase = EGamePhase::Processing;
}

void ASchemeGameMode::ProcessBlockRequest(ASchemePlayerController* Blocker)
{
	if (CurrentPhase != EGamePhase::ActionReaction) return;
	
	// If the action is not blockable, ignore the block request
	if (!CurrentContext.ActionData->BlockableByCard) return;
	
	CurrentContext.Blocker = Blocker;
	// CurrentContext.ClaimedBlockCard = BlockCard;
	
	BroadcastNotificationPacket(
		FNotificationPacket{ServerNotificationMap.Find(EServerNotificationType::TimeoutNotification)->Get(),
		FText::FromString("An action block has been declared. Blocker claims it has the card: " + CurrentContext.ActionData->BlockableByCard->GetCardName().ToString())}
	);
	
	SetGamePhase(EGamePhase::BlockReaction);
}

void ASchemeGameMode::ExecuteCurrentAction()
{
	if (!bIsGameStarted) return;

	if (CurrentContext.ActionData && CurrentContext.ActionData->ActionLogicClass)
	{
		UBaseAction* ActionInstance = NewObject<UBaseAction>(this, CurrentContext.ActionData->ActionLogicClass);
		if (!ActionInstance) return;
		ActionInstance->ExecuteAction(CurrentContext.InstigatorCont, CurrentContext.Target);
	}

	CheckPlayerCondition(CurrentContext.Target);

	SetGamePhase(EGamePhase::TurnEnd);
}

void ASchemeGameMode::FinalizeTurn()
{
	if (!bIsGameStarted) return;

	if (ASchemeGameState* SchemeGameState = GetWorld()->GetGameState<ASchemeGameState>())
	{
		SchemeGameState->Server_AdvanceToNextPlayerTurn(CurrentContext.InstigatorCont);
		
		BroadcastNotificationPacket(
			FNotificationPacket{ServerNotificationMap.Find(EServerNotificationType::GeneralNotification)->Get(),
			FText::FromString("Turn has ended. Next player's turn begins.")}
		);
	}
	CurrentContext.Reset();
	CurrentPhase = EGamePhase::Idle;
}

void ASchemeGameMode::CheckPlayerCondition(const ASchemePlayerController* CheckPlayer)
{
	if (!CheckPlayer) return;
	
	// Check if there are any cards left
	if (!CheckPlayer->HasAnyCardInHand())
	{
		ASchemePlayerState* SchemePlayerState = CheckPlayer->GetPlayerState<ASchemePlayerState>();
		if (!SchemePlayerState) return;
		
		SchemePlayerState->SetIsEliminated(true);
		BroadcastNotificationPacket(FNotificationPacket{
			ServerNotificationMap.Find(EServerNotificationType::GeneralNotification)->Get(),
			FText::FromString("Player ( " + SchemePlayerState->GetUsername().ToString() + "eliminated from the game due to losing all cards.")}
		);
		
		CheckWinCondition();
	}	
}

void ASchemeGameMode::CheckWinCondition()
{
	int32 ActivePlayers = 0;
	APlayerController* PotentialWinner = nullptr;
	for (APlayerController* PlayerController : CurrentPlayers)
	{
		if (!PlayerController) continue;
		ASchemePlayerState* PlayerState = PlayerController->GetPlayerState<ASchemePlayerState>();
		if (PlayerState && !PlayerState->IsEliminated())
		{
			ActivePlayers++;
			PotentialWinner = PlayerController;
		}
	}
	if (ActivePlayers == 1 && PotentialWinner)
	{
		EndSchemeGame(Cast<ASchemePlayerController>(PotentialWinner));
	}
	else if (ActivePlayers == 0 && bIsGameStarted)
	{
		// Everyone left or eliminated?
		ResetGame();
	}
	
}

void ASchemeGameMode::EndSchemeGame(const ASchemePlayerController* WinnerController)
{
	BroadcastNotificationPacket(
		FNotificationPacket{ServerNotificationMap.Find(EServerNotificationType::GeneralNotification)->Get(),
		FText::FromString("The game has ended! The winner is " + WinnerController->GetPlayerState<ASchemePlayerState>()->GetUsername().ToString() + ".")}
	);
	BroadcastNotificationPacket(
		FNotificationPacket{ServerNotificationMap.Find(EServerNotificationType::GameEndNotification)->Get(),
		FText::FromString(WinnerController->GetPlayerState<ASchemePlayerState>()->GetUsername().ToString())}
	);
	
	bIsGameStarted = false;
	
	// Restart game after 10 seconds
	FTimerHandle RestartHandle;
	GetWorldTimerManager().SetTimer(RestartHandle, this, &ASchemeGameMode::ResetGame, 10.f, false);
}

void ASchemeGameMode::ResetGame()
{
	bIsGameStarted = false;
	bCanGameStart = (CurrentPlayers.Num() >= MinPlayer && CurrentPlayers.Num() <= MaxPlayer);
	CurrentPhase = EGamePhase::Idle;
	CurrentContext.Reset();
	CurrentTurnIndex = 0;
	
	// Reset GameState
	if (ASchemeGameState* SGS = GetGameState<ASchemeGameState>())
	{
		SGS->ResetState();
	}
	
	// Reset Players
	for (APlayerController* PC : CurrentPlayers)
	{
		if (ASchemePlayerController* SPC = Cast<ASchemePlayerController>(PC))
		{
			SPC->ResetState();
		}
		if (ASchemePlayerState* SPS = PC->GetPlayerState<ASchemePlayerState>())
		{
			SPS->ResetState();
		}
	}
}

void ASchemeGameMode::SetGamePhase(EGamePhase NewPhase)
{
	if (!bIsGameStarted) return;

	CurrentPhase = NewPhase;
	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	if (NewPhase == EGamePhase::ActionReaction)
	{
		FText PlayerName = FText::FromString("Player");
		if (ASchemePlayerState* PlayerState = CurrentContext.InstigatorCont->GetPlayerState<ASchemePlayerState>())
			PlayerName = PlayerState->GetUsername();
		
		if (CurrentContext.ActionData->RequiredCardToPerform)
		{
			BroadcastNotificationPacket(
				FNotificationPacket{ServerNotificationMap.Find(EServerNotificationType::ChallengeNotification)->Get(),
				FText::FromString(PlayerName.ToString() + " has performed " + CurrentContext.ActionData->ActionName.ToString() + ". You have " + FString::SanitizeFloat(ReactionTime) + " seconds to respond.")},
				CurrentContext.InstigatorCont
				);
				
		}
		if (CurrentContext.ActionData->BlockableByCard)
		{
			BroadcastNotificationPacket(
				FNotificationPacket{ServerNotificationMap.Find(EServerNotificationType::BlockNotification)->Get(),
				FText::FromString(PlayerName.ToString() + " has performed " + CurrentContext.ActionData->ActionName.ToString() + ". You have " + FString::SanitizeFloat(ReactionTime) + " seconds to respond.")},
				CurrentContext.InstigatorCont
				);
		}
		GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ASchemeGameMode::OnPhaseTimerExpired, ReactionTime, false);
	}
	else if (NewPhase == EGamePhase::BlockReaction)
	{
		FText PlayerName = FText::FromString("Player");
		if (ASchemePlayerState* PlayerState = CurrentContext.Blocker->GetPlayerState<ASchemePlayerState>())
			PlayerName = PlayerState->GetUsername();
		if (CurrentContext.ActionData->BlockableByCard)
		{
			BroadcastNotificationPacket(
				FNotificationPacket{ServerNotificationMap.Find(EServerNotificationType::ChallengeNotification)->Get(),
				FText::FromString(PlayerName.ToString() + " has performed a block. Claiming has the card: " + CurrentContext.ActionData->BlockableByCard->GetCardName().ToString() + ". You have " + FString::SanitizeFloat(ReactionTime) + " seconds to respond.")},
				CurrentContext.Blocker
				);
		}
		GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ASchemeGameMode::OnPhaseTimerExpired, ReactionTime, false);
	}
	else if (NewPhase == EGamePhase::TurnEnd)
	{
		FinalizeTurn();
	}
	
}

void ASchemeGameMode::OnPhaseTimerExpired()
{
	if (CurrentPhase == EGamePhase::ActionReaction)
	{
		BroadcastNotificationPacket(
			FNotificationPacket{ServerNotificationMap.Find(EServerNotificationType::TimeoutNotification)->Get(),
			FText::FromString("Action challenge period has timed out.")},
			CurrentContext.InstigatorCont
			);
		
		// No challenge to action. Execute it.
		ExecuteCurrentAction();
		SetGamePhase(EGamePhase::TurnEnd);
		return;
	}
	if (CurrentPhase == EGamePhase::BlockReaction)
	{
		BroadcastNotificationPacket(
			FNotificationPacket{ServerNotificationMap.Find(EServerNotificationType::TimeoutNotification)->Get(),
			FText::FromString("Block challenge period has timed out.")},
			CurrentContext.Blocker
			);
		
		SetGamePhase(EGamePhase::TurnEnd);
	}
}

void ASchemeGameMode::AutoResolveChallenge(ASchemePlayerController* Accused, ASchemePlayerController* Challenger,
	const UCardDataAsset* RequiredCard)
{
	if (!Accused || !Challenger || !RequiredCard) return;
	
	if (Accused->HasCardInHand(RequiredCard))
	{
		// Accused wins the challenge (blocker or action dealer)
		
		// Remove the revealed card from hand then draw another card (accused)
		SwapCardForPlayer(Accused, Accused->GetCardFromHand(RequiredCard->GetCardRole()));
		
		ApplyPenalty(Challenger);
		
		if (CurrentContext.Blocker == nullptr)
		{
			BroadcastNotificationPacket(
				FNotificationPacket{ServerNotificationMap.Find(EServerNotificationType::GeneralNotification)->Get(),
				FText::FromString("The challenge failed, the action proceeds.")}
			);
			
			// It was an action challenge, action proceeds (not block)
			ExecuteCurrentAction();
		}
		else
		{
			BroadcastNotificationPacket(
				FNotificationPacket{ServerNotificationMap.Find(EServerNotificationType::GeneralNotification)->Get(),
				FText::FromString("The block has succeeded, the action is blocked.")}
			);
			
			// It was a block challenge, action fails (blocked)
			SetGamePhase(EGamePhase::TurnEnd);
		}
	}
	else
	{
		// Challenger wins the challenge
		ApplyPenalty(Accused);
		
		if (CurrentContext.Blocker != nullptr && Accused == CurrentContext.Blocker)
		{
			// Block failed, execute action
			BroadcastNotificationPacket(
				FNotificationPacket{ServerNotificationMap.Find(EServerNotificationType::GeneralNotification)->Get(),
				FText::FromString("The block has failed, the action is executed.")}
			);
			ExecuteCurrentAction();
		}
		else
		{
			// Challenge succeed, stop action
			BroadcastNotificationPacket(
				FNotificationPacket{ServerNotificationMap.Find(EServerNotificationType::GeneralNotification)->Get(),
				FText::FromString("The challenge succeeded, the action is blocked.")}
			);
			
			SetGamePhase(EGamePhase::TurnEnd);
		}
	}
}

void ASchemeGameMode::ApplyPenalty(ASchemePlayerController* Victim)
{
	if (!Victim) return;
	
	if (Victim->HasAnyCardInHand())
	{
		Victim->Server_RemoveRandomCardFromHand();
		BroadcastNotificationPacket(FNotificationPacket{ServerNotificationMap.Find(EServerNotificationType::GeneralNotification)->Get(),
		FText::FromString(Victim->GetPlayerState<ASchemePlayerState>()->GetUsername().ToString() + " has lost a card due to losing the challenge.")});

	}
	
	CheckPlayerCondition(Victim);
}

void ASchemeGameMode::SwapCardForPlayer(ASchemePlayerController* Player, const ACardActor* CardToSwap)
{
	if (!Player || !CardToSwap) return;
	
	Player->Server_RemoveCardFromHand(CardToSwap->GetCardData());
	DrawCard(Player, Player->GetFirstEmptyCardHoldingPointIndex());
}

void ASchemeGameMode::OnTurnChanged(ASchemePlayerState* NewTurnPlayer)
{
	if (!NewTurnPlayer) return;

	if (ForcedActionData && NewTurnPlayer->ShouldForceAct())
	{
		if (ASchemePlayerController* PC = Cast<ASchemePlayerController>(NewTurnPlayer->GetPlayerController()))
		{
			SendGeneralNotificationToPlayer(PC, FText::FromString("You have over 10 gold! You must use the forced action this turn."));
		}
	}
}

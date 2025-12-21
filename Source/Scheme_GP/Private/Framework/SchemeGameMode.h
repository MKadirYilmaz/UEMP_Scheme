// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Gameplay/Action/SchemeNotification.h"
#include "SchemeGameMode.generated.h"

class UActionDataAsset;
class ASchemePlayerController;
class ACardActor;
class UCardDataAsset;
class APlayerStart;

UENUM(BlueprintType)
enum class EServerNotificationType : uint8
{
	ChallengeNotification,
	TimeoutNotification,
	GeneralNotification,
	GameStartNotification,
	GameEndNotification
};


/**
 * 
 */
UCLASS()
class ASchemeGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;
	
public:
	// Called when a new player joins (Only on server)
	virtual void PostLogin(APlayerController* NewPlayer) override;
	// Called when a player leaves (Only on server)
	virtual void Logout(AController* Exiting) override;
	
	// Process a player's action request
	void ProcessPlayerAction(ASchemePlayerController* RequestingController, UActionDataAsset* ActionData, ASchemePlayerController* TargetController);
	// Notify all players about a player's action
	void BroadcastPlayerActionNotification(ASchemePlayerController* ActionDealer, ASchemePlayerController* ActionTarget, FNotificationPacket& Packet);
	// Notify all players about timeout event
	void BroadcastTimeoutNotification();
	
	void BroadcastNotificationPacket(const FNotificationPacket& Packet);
	
	void SendGeneralNotificationToPlayer(ASchemePlayerController* TargetPlayer, const FText& Message);
	
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	
	void BreakTimeoutCountdown();
	
	UFUNCTION(BlueprintCallable)
	void ProcessChallengeRequest();
	
	UFUNCTION(BlueprintCallable)
	void CreateVirtualDeck();
	
	/**
	 * Randomizes the order of the cards in the virtual deck.
	 *
	 * This method rearranges the elements in the VirtualGameDeck array to achieve
	 * a randomized order by transferring all cards to a temporary array and adding
	 * them back into VirtualGameDeck in a random sequence. The random index for each
	 * card selection is determined using the UKismetMathLibrary::RandomInteger function.
	 *
	 * Note:
	 * - Ensure that VirtualGameDeck is initialized and populated before calling this method.
	 * - After execution, VirtualGameDeck will contain the same cards but in a shuffled order.
	 */
	UFUNCTION(BlueprintCallable)
	void ShuffleDeck();

	/**
	 * Deals an initial set of cards to each player in the game.
	 *
	 * This method iterates through the specified number of cards per player and distributes
	 * one card at a time to all players currently listed in the GameState's PlayerArray. For
	 * each player, the DrawCard method is called, which handles the process of drawing a card
	 * for a specific player's state.
	 *
	 * Note:
	 * - Ensure that the GameState and its PlayerArray are properly initialized before calling this method.
	 * - The CardsPerPlayer parameter determines how many cards each player will receive at the start.
	 * - Only player states of type ASchemePlayerState will have cards dealt to them.
	 *
	 * @param CardsPerPlayer The number of cards each player will receive as their initial hand.
	 * This value must be greater than 0 to distribute cards.
	 */
	UFUNCTION(BlueprintCallable)
	void DealInitialCards(int32 CardsPerPlayer);
	
	UFUNCTION(BlueprintCallable)
	void DrawCard(ASchemePlayerController* PlayerController, int32 HoldIndex);
	UFUNCTION(BlueprintCallable)
	void ReturnCardToDeck(AActor* CardToReturn);
	
	
	UFUNCTION(BlueprintCallable)
	void StartSchemeGame();

	UFUNCTION(BlueprintImplementableEvent)
	void OnCanGameStartValid();
private:
	void FindAllStartLocations();
private:
	UPROPERTY(EditDefaultsOnly, Category = "Card System")
	TArray<UCardDataAsset*> AllCardDataTypes;
	UPROPERTY(VisibleAnywhere, Category = "Card System")
	TArray<UCardDataAsset*> VirtualGameDeck;
	UPROPERTY(EditDefaultsOnly, Category = "Card System")
	TSubclassOf<ACardActor> CardActorClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Notification System")
	TMap<EServerNotificationType, TSubclassOf<USchemeNotification>> ServerNotificationMap;
	
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Adjustments")
	int32 MinPlayer = 2;
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Adjustments")
	int32 MaxPlayer = 8;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Adjustments")
	int32 CardAmountPerPlayer = 2;
	
	UPROPERTY()
	TArray<APlayerController*> CurrentPlayers;
	UPROPERTY(VisibleAnywhere, Category = "Spawn System")
	TArray<APlayerStart*> PlayerStartLocations;

	bool bCanGameStart = false;
	bool bIsGameStarted = false;
	
	bool bChallengeRequest = false;
	FTimerHandle ChallengeTimeoutHandle;
	
	UPROPERTY()
	UActionDataAsset* CurrChallengeAction;
	UPROPERTY()
	ASchemePlayerController* CurrActionController;
	UPROPERTY()
	ASchemePlayerController* CurrTargetController;

	int32 CurrentTurnIndex = 0;
public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetCanGameStart() const { return bCanGameStart; }
	
};

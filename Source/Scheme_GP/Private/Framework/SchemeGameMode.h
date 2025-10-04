// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SchemeGameMode.generated.h"

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
	
	UFUNCTION(Server, Reliable, Category = "Gold System")
	void TryProcessGoldIncome(APlayerController* RequestingController, int32 Amount);
	UFUNCTION(Server, Reliable, Category = "Gold System")
	void TryProcessGoldOutcome(APlayerController* RequestingController, int32 Amount);

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	
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
	void DrawCard(class ASchemePlayerState* PlayerState);
	
	UFUNCTION(BlueprintCallable)
	void StartSchemeGame();

	UFUNCTION(BlueprintImplementableEvent)
	void OnCanGameStartValid();
private:

	void FindAllStartLocations();
private:
	UPROPERTY(EditDefaultsOnly, Category = "Card System")
	TArray<class UCardDataAsset*> AllCardDataTypes;
	UPROPERTY(VisibleAnywhere, Category = "Card System")
	TArray<class UCardDataAsset*> VirtualGameDeck;
	
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Adjustments")
	int32 MinPlayer = 2;
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Adjustments")
	int32 MaxPlayer = 8;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Adjustments")
	int32 CardAmountPerPlayer = 2;
	
	TArray<APlayerController*> CurrentPlayers;
	TArray<class APlayerStart*> PlayerStartLocations;

	bool bCanGameStart = false;
	bool bIsGameStarted = false;

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetCanGameStart() const { return bCanGameStart; }
	
};

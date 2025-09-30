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

public:
	
	bool TryProcessGoldIncome(APlayerController* RequestingController, int32 Amount);
	bool TryProcessGoldOutcome(APlayerController* RequestingController, int32 Amount);

	/**
	 * Creates a virtual deck of cards based on the number of players in the game.
	 *
	 * This method initializes and populates the VirtualGameDeck array by adding multiple
	 * copies of each card from the AllCardDataTypes collection. The number of copies per
	 * card is determined by the number of players. For up to 7 players, 3 copies per card
	 * are added, and for more than 7 players, 4 copies per card are added.
	 *
	 * Note:
	 * - Ensure that GameState and AllCardDataTypes are properly initialized before calling this method.
	 * - The VirtualGameDeck array will be emptied before being populated.
	 * - If GameState is invalid, the method will exit without modifying the deck.
	 *
	 * @param NumOfPlayers The number of players participating in the game, which impacts
	 * the number of copies added per card. If not provided explicitly, it will be updated
	 * based on the current count of players in GameState's PlayerArray.
	 */
	UFUNCTION(BlueprintCallable)
	void CreateVirtualDeck(int32 NumOfPlayers);
	
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
	

private:
	UPROPERTY(EditDefaultsOnly, Category = "Card System")
	TArray<class UCardDataAsset*> AllCardDataTypes;
	UPROPERTY(VisibleAnywhere, Category = "Card System")
	TArray<class UCardDataAsset*> VirtualGameDeck;
};

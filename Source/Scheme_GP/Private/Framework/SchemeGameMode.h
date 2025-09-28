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

	UFUNCTION(BlueprintCallable)
	void CreateVirtualDeck(int32 NumOfPlayers);
	UFUNCTION(BlueprintCallable)
	void ShuffleDeck();
	UFUNCTION(BlueprintCallable)
	void DrawCard(APawn* DrawingPawn);
	

private:
	UPROPERTY(EditDefaultsOnly, Category = "Card System")
	TArray<class UCardDataAsset*> AllCardDataTypes;
	UPROPERTY(VisibleAnywhere, Category = "Card System")
	TArray<class UCardDataAsset*> VirtualGameDeck;
};

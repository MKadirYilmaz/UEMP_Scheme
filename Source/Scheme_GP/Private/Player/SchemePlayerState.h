// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SchemePlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGoldChanged, ASchemePlayerState*, PlayerState, int32, NewGold, int32, Delta);
/**
 * 
 */
UCLASS()
class ASchemePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	// Do not call this function other than GameMode class
	void AddGold(int32 amount);
	// Do not call this function other than GameMode class
	bool RemoveGold(int32 amount);
	// Do not call this function other than GameMode class
	void AddCardToHand(class UCardDataAsset* NewCard);

	UFUNCTION(BlueprintPure, Category = "Gold")
	FORCEINLINE int32 GetGold() const { return Gold; }

	UPROPERTY(BlueprintAssignable, Category = "Gold")
	FOnGoldChanged OnGoldChanged;

private:
	UPROPERTY(VisibleAnywhere, Category = "Gold")
	int32 Gold = 0;
	UPROPERTY(VisibleAnywhere, Category = "Card")
	TArray<class UCardDataAsset*> HoldingCards;
	
	
};

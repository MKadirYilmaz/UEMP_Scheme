// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SchemePlayerState.generated.h"

/**
 * 
 */
UCLASS()
class ASchemePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ASchemePlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Do not call this function other than GameMode class
	void AddGold(int32 amount);
	// Do not call this function other than GameMode class
	bool RemoveGold(int32 amount);
	// Do not call this function other than GameMode class
	void AddCardToHand(class UCardDataAsset* NewCard);

	UFUNCTION(BlueprintPure, Category = "Gold")
	FORCEINLINE int32 GetGold() const { return Gold; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Gold")
	void OnGoldChange(int32 NewGold, int32 Delta);
private:
	UPROPERTY(ReplicatedUsing = OnRep_Gold, VisibleAnywhere, Category = "Gold")
	int32 Gold = 0;
	UPROPERTY(VisibleAnywhere, Category = "Card")
	TArray<class UCardDataAsset*> HoldingCards;

	// Called only on clients when the value has changed
	UFUNCTION()
	void OnRep_Gold();
};

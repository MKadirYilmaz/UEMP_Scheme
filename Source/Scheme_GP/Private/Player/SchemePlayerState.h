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

protected:
	virtual void BeginPlay() override;
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
	UFUNCTION(BlueprintImplementableEvent, Category = "Card")
	void OnCardChange(const TArray<class UCardDataAsset*>& NewCards);

	FTransform GetNextCardHoldingPoint();
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Card")
	TArray<USceneComponent*> CardHoldingPoints;
	UPROPERTY(EditDefaultsOnly, Category = "Card System")
	TSubclassOf<class AActor> CardTableClass;

	UPROPERTY()
	class ACardTable* CardTable;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerIndex, VisibleAnywhere)
	int32 PlayerIndex = 0;
	int32 CardHoldingPointIndex = 0;
	
	UPROPERTY(ReplicatedUsing = OnRep_Gold, VisibleAnywhere, Category = "Gold")
	int32 Gold = 0;
	UPROPERTY(ReplicatedUsing = OnRep_HoldingCards, VisibleAnywhere, Category = "Card")
	TArray<class UCardDataAsset*> HoldingCards;

	int32 CachedDelta = 0;
	
	// Called only on clients when the value has changed
	UFUNCTION()
	void OnRep_Gold();
	// Called only on clients when the value has changed
	UFUNCTION()
	void OnRep_HoldingCards();
	// Called only on clients when the value has changed
	UFUNCTION()
	void OnRep_PlayerIndex();

public:
	FORCEINLINE void SetPlayerIndex(int32 NewIndex) { PlayerIndex = NewIndex; }
};

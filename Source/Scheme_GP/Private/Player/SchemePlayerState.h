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
	
	// Do not call this function other than server
	void AddGold(int32 amount);
	// Do not call this function other than server
	bool RemoveGold(int32 amount);
	
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Gold")
	void OnGoldChange(int32 NewGold, int32 Delta);

protected:

	UPROPERTY(ReplicatedUsing = OnRep_PlayerIndex, VisibleAnywhere)
	int32 PlayerIndex = 0;
	UPROPERTY(ReplicatedUsing = OnRep_Username, VisibleAnywhere)
	FText Username;
	UPROPERTY(ReplicatedUsing = OnRep_PlayerIndex, VisibleAnywhere)
	bool bIsEliminated = false;
	
	
	UPROPERTY(ReplicatedUsing = OnRep_Gold, VisibleAnywhere, Category = "Gold")
	int32 Gold = 0;

	int32 CachedDelta = 0;
	
	// Called only on clients when the value has changed
	UFUNCTION()
	void OnRep_Gold();
	
	UFUNCTION()
	void OnRep_Username();
	
	// Called only on clients when the value has changed
	UFUNCTION()
	void OnRep_PlayerIndex();
	UFUNCTION()
	void OnRep_IsEliminated();

public:
	FORCEINLINE void SetPlayerIndex(int32 NewIndex) { PlayerIndex = NewIndex; }
	FORCEINLINE int32 GetPlayerIndex() const { return PlayerIndex; }
	
	FORCEINLINE bool IsEliminated() const { return bIsEliminated; }
	FORCEINLINE void SetIsEliminated(bool bEliminated) { bIsEliminated = bEliminated; }
	
	FORCEINLINE void SetUsername(const FText& NewUsername) { Username = NewUsername; }
	FORCEINLINE FText GetUsername() const { return Username; }
	
	UFUNCTION(BlueprintPure, Category = "Gold")
	FORCEINLINE int32 GetGold() const { return Gold; }
	
};

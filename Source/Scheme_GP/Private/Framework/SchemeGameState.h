// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SchemeGameState.generated.h"

class ASchemePlayerController;
class ASchemePlayerState;
/**
 * 
 */
UCLASS()
class ASchemeGameState : public AGameStateBase
{
	GENERATED_BODY()

protected:
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION()
	void OnRep_CurrentPlayerTurn();
	UFUNCTION()
	void OnRep_PlayerTurnsOrder();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_AdvanceToNextPlayerTurn(ASchemePlayerController* RequestingController);
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_ChangePlayerGoldCount(ASchemePlayerState* RequestingPlayerState, int32 Amount);
	
	UFUNCTION(BlueprintCallable)
	void StealGoldFromPlayer(ASchemePlayerState* FromPlayer, ASchemePlayerState* ToPlayer, int32 Amount);
	
public:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentPlayerTurn, VisibleAnywhere, BlueprintReadOnly, Category = "Player Turn")
	APlayerState* CurrentPlayerTurn;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentPlayerTurn, VisibleAnywhere, BlueprintReadOnly, Category = "Player Turn")
	TArray<APlayerState*> PlayerTurnsOrder;



	
};

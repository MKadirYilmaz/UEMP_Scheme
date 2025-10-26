// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SchemeGameState.generated.h"

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

public:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentPlayerTurn, VisibleAnywhere, BlueprintReadOnly, Category = "Player Turn")
	class APlayerState* CurrentPlayerTurn;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentPlayerTurn, VisibleAnywhere, BlueprintReadOnly, Category = "Player Turn")
	TArray<class APlayerState*> PlayerTurnsOrder;



	
};

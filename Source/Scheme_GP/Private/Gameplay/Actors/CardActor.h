// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Actors/BaseInteractableActor.h"
#include "CardActor.generated.h"

/**
 * 
 */
UCLASS()
class ACardActor : public ABaseInteractableActor
{
	GENERATED_BODY()


protected:
	UPROPERTY(EditDefaultsOnly, Category = "Card Data")
	class UCardDataAsset* CardData;

public:
	FORCEINLINE class UCardDataAsset* GetCardData() const { return CardData; }
};

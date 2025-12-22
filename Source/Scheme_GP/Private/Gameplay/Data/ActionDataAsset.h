// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ActionDataAsset.generated.h"

class UBaseAction;
class UCardDataAsset;

/**
 * 
 */
UCLASS()
class UActionDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action Config")
	FText ActionName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action Config")
	FText ActionDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action Rules")
	int32 GoldCost;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action Logic")
	TSubclassOf<UBaseAction> ActionLogicClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action Logic")
	UCardDataAsset* RequiredCardToPerform;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action Logic")
	UCardDataAsset* BlockableByCard;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ActionDataAsset.generated.h"

class UBaseAction;

UENUM(BlueprintType)
enum class EActionTargetType : uint8
{
	Self            UMETA(DisplayName = "Self"),
	SingleOpponent  UMETA(DisplayName = "Single Opponent"),
	AllOpponents    UMETA(DisplayName = "All Opponents")
};
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action Rules")
	bool bCanBeInterrupted;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action Rules")
	EActionTargetType TargetType;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action Logic")
	TSubclassOf<UBaseAction> ActionLogicClass;
};

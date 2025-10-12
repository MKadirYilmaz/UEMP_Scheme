// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CardDataAsset.generated.h"

UENUM(BlueprintType)
enum class ECardRole : uint8
{
	Merchant,
	Knight,
	Steward,
	Counselor,
	GuildMaster,
	// Other roles
	None
};

/**
 * 
 */
UCLASS()
class UCardDataAsset : public UDataAsset
{
	GENERATED_BODY()

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card Data")
	ECardRole CardRole;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card Data")
	FText CardName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card Data")
	FText CardDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card Visual")
	UMaterialInstance* CardMaterial;

public:
	FORCEINLINE ECardRole GetCardRole() const { return CardRole; }
	FORCEINLINE FText GetCardName() const { return CardName; }
	FORCEINLINE FText GetCardDescription() const { return CardDescription; }
	FORCEINLINE UMaterialInstance* GetCardMaterial() const { return CardMaterial; }
	//
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Actors/BaseInteractableActor.h"
#include "CardActor.generated.h"

class UCardDataAsset;
/**
 * 
 */
UCLASS()
class ACardActor : public ABaseInteractableActor
{
	GENERATED_BODY()

public:
	ACardActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetCardData(UCardDataAsset* NewData);
	UFUNCTION()
	void OnRep_CardData();
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Card Visual")
	UStaticMeshComponent* CardMesh;

	UPROPERTY(ReplicatedUsing = OnRep_CardData, VisibleAnywhere, Category = "Card Data")
	UCardDataAsset* CardData = nullptr;

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE UCardDataAsset* GetCardData() const { return CardData; }

};

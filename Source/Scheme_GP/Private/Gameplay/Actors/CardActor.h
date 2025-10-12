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

public:
	ACardActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetCardData(class UCardDataAsset* NewData);
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Card Visual")
	UStaticMeshComponent* CardMesh;

	UPROPERTY(ReplicatedUsing = OnRep_CardData, VisibleAnywhere, Category = "Card Data")
	class UCardDataAsset* CardData;

	UFUNCTION()
	void OnRep_CardData();

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE class UCardDataAsset* GetCardData() const { return CardData; }

};

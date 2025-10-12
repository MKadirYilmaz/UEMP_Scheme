// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CardTable.generated.h"

USTRUCT()
struct FCardLocationStruct
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere)
	TArray<FTransform> CardTransforms;
};
UCLASS()
class ACardTable : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	ACardTable();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Construction")
	void CalculateCardLocationPairs();

private:
	UPROPERTY(EditAnywhere, Category = "Card Table Adjustments")
	UStaticMeshComponent* TableMesh;

	UPROPERTY(EditAnywhere, Category = "Card Table Adjustments")
	float CardPointsRotationOffset = 11.25f;

	UPROPERTY(EditAnywhere, Category = "Card Game")
	TArray<FCardLocationStruct> CardPointsStructs;

public:
	FORCEINLINE TArray<FCardLocationStruct> GetCardPointsStructs() const { return CardPointsStructs; }
};

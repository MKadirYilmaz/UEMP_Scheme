// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryPrimaryInteract();

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Adjustments")
	float MaxInteractionDistance = 100.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Adjustments")
	TEnumAsByte<ECollisionChannel> LineTraceChannel = ECC_Visibility;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Adjustments")
	bool bDrawLineTraceLine = true;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Adjustments")
	USceneComponent* LineTraceStartComp;


	AActor* HoveredActor;
	APawn* OwnerPawn;

public:
	FORCEINLINE void SetLineTraceStartComp(USceneComponent* NewLineTraceStartComp) { LineTraceStartComp = NewLineTraceStartComp; }
};

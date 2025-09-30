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

	/**
	 * Performs a line trace in the world to detect interactable objects.
	 *
	 * This function casts a line trace starting from the location of the LineTraceStartComp
	 * component in the direction of its forward vector, up to a maximum distance defined
	 * by MaxInteractionDistance. The line trace uses the collision channel specified
	 * in LineTraceChannel.
	 *
	 * If the trace hits an actor that implements the UInteractableInterface, the OnInteract
	 * function of the interface is executed with the player controller as a parameter.
	 * Returns the hit result of the line trace. If no valid interactable target is hit,
	 * an empty FHitResult is returned.
	 *
	 * @return The result of the line trace. Will contain information about the hit actor and
	 *         impact point if a valid interactable object is detected.
	 */
	FHitResult CheckInteractionLineTrace();
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

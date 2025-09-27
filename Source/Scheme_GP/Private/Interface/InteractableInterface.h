// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class IInteractableInterface
{
	GENERATED_BODY()

public:
	/**
	 * Handles the logic when an actor gains focus from an interactor.
	 *
	 * @param Interactor The pawn that focuses on the actor.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnBeginFocus(APawn* Interactor);


	/**
	 * Handles the logic when an actor loses focus from an interactor.
	 *
	 * @param Interactor The pawn that has lost focus on the actor.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnEndFocus(APawn* Interactor);


	/**
	 * Handles the interaction logic when an interactor interacts with the actor.
	 *
	 * @param Interactor The pawn that initiates the interaction with the actor.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnInteract(APawn* Interactor);
	
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
};

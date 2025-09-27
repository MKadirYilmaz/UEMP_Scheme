// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
#include "BaseInteractableActor.generated.h"

UCLASS()
class ABaseInteractableActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseInteractableActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnBeginFocus_Implementation(APawn* Interactor) override;
	virtual void OnEndFocus_Implementation(APawn* Interactor) override;
	virtual void OnInteract_Implementation(APawn* Interactor) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

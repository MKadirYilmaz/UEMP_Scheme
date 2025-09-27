// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Actors/BaseInteractableActor.h"

// Sets default values
ABaseInteractableActor::ABaseInteractableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseInteractableActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseInteractableActor::OnBeginFocus_Implementation(APawn* Interactor)
{
	IInteractableInterface::OnBeginFocus_Implementation(Interactor);
}

void ABaseInteractableActor::OnEndFocus_Implementation(APawn* Interactor)
{
	IInteractableInterface::OnEndFocus_Implementation(Interactor);
}

void ABaseInteractableActor::OnInteract_Implementation(APawn* Interactor)
{
	IInteractableInterface::OnInteract_Implementation(Interactor);
}

// Called every frame
void ABaseInteractableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


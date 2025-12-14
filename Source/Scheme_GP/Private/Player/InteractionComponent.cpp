// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/InteractionComponent.h"

#include "SchemePlayerController.h"
#include "Interface/InteractableInterface.h"

// Sets default values for this component's properties
UInteractionComponent::UInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPawn = Cast<APawn>(GetOwner());
	
}


// Called every frame
void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (LineTraceStartComp == nullptr || !OwnerPawn->IsLocallyControlled())
		return;
	
	FHitResult HitResult;
	FVector Start = LineTraceStartComp->GetComponentLocation();
	FVector End = Start + LineTraceStartComp->GetForwardVector() * MaxInteractionDistance;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, LineTraceChannel))
	{
		if (bDrawLineTraceLine) DrawDebugLine(GetWorld(), Start, HitResult.ImpactPoint, FColor::Red, false, 0.f, 0, 0.5f);

		if (HitResult.GetActor()->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
		{
			// Check if we are looking at another actor
			if (HoveredActor != HitResult.GetActor())
			{
				if (HoveredActor)
				{
					UE_LOG(LogTemp, Display, TEXT("End Hovered Actor: %s"), *HoveredActor->GetName());
					IInteractableInterface::Execute_OnEndFocus(HoveredActor, OwnerPawn);
				}
				UE_LOG(LogTemp, Display, TEXT("Start Hovered Actor: %s"), *HitResult.GetActor()->GetName());
				IInteractableInterface::Execute_OnBeginFocus(HitResult.GetActor(), OwnerPawn);
			}
			HoveredActor = HitResult.GetActor();
		}
		else if (HoveredActor)
		{
			UE_LOG(LogTemp, Display, TEXT("End Hovered Actor: %s"), *HoveredActor->GetName());
			IInteractableInterface::Execute_OnEndFocus(HoveredActor, OwnerPawn);
			HoveredActor = nullptr;
		}
	}
	else
	{
		if (bDrawLineTraceLine) DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.f, 0, 0.5f);
		if (!HoveredActor)
			return;
		UE_LOG(LogTemp, Display, TEXT("End Hovered Actor"));
		IInteractableInterface::Execute_OnEndFocus(HoveredActor, OwnerPawn);
		HoveredActor = nullptr;
	}
}

void UInteractionComponent::TryPrimaryInteract()
{
	if (!LineTraceStartComp || !OwnerPawn)
		return;
	if (!OwnerPawn->IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("Only the local player can interact!"));
		return;
	}
	if (!HoveredActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Actor Hovered!"));
		return;
	}
	if (ASchemePlayerController* PlayerController = Cast<ASchemePlayerController>(OwnerPawn->GetController()))
	{
		PlayerController->Server_RequestInteract(HoveredActor, OwnerPawn);
		UE_LOG(LogTemp, Display, TEXT("Primary Interact Server Permission Requested By %s"), *OwnerPawn->GetName());
	}
}




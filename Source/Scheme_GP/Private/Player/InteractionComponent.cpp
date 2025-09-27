// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/InteractionComponent.h"

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
	if (LineTraceStartComp == nullptr)
		return;
	
	FHitResult HitResult;
	FVector Start = LineTraceStartComp->GetComponentLocation();
	FVector End = Start + LineTraceStartComp->GetForwardVector() * MaxInteractionDistance;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, LineTraceChannel))
	{
		if (bDrawLineTraceLine) DrawDebugLine(GetWorld(), Start, HitResult.ImpactPoint, FColor::Red, false, 0.f, 0, 0.5f);

		if (HitResult.GetActor()->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Inspecting on: %s"), *HitResult.GetActor()->GetName());
			// Check if we are looking at another actor
			if (HoveredActor != HitResult.GetActor())
			{
				IInteractableInterface::Execute_OnBeginFocus(HitResult.GetActor(), OwnerPawn);
				if (HoveredActor)
					IInteractableInterface::Execute_OnEndFocus(HoveredActor, OwnerPawn);
			}
			HoveredActor = HitResult.GetActor();
		}
		else if (HoveredActor)
		{
			IInteractableInterface::Execute_OnEndFocus(HoveredActor, OwnerPawn);
			HoveredActor = nullptr;
		}
	}
	else
	{
		if (bDrawLineTraceLine) DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.f, 0, 0.5f);
		if (!HoveredActor)
			return;
		IInteractableInterface::Execute_OnEndFocus(HoveredActor, OwnerPawn);
		HoveredActor = nullptr;
	}
}

FHitResult UInteractionComponent::CheckInteractionLineTrace()
{
	if (LineTraceStartComp == nullptr)
		return FHitResult();
	
	FHitResult HitResult;
	FVector Start = LineTraceStartComp->GetComponentLocation();
	FVector End = Start + LineTraceStartComp->GetForwardVector() * MaxInteractionDistance;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, LineTraceChannel))
	{
		if (HitResult.GetActor()->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
		{
			IInteractableInterface::Execute_OnInteract(HitResult.GetActor(), OwnerPawn);
			return HitResult;
		}
		else
		{
			return FHitResult();
		}
	}
	return HitResult;
}




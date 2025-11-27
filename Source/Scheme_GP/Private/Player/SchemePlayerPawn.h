// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Interface/InteractableInterface.h"
#include "SchemePlayerPawn.generated.h"

class UCameraComponent;
class ASchemePlayerController;
UCLASS()
class ASchemePlayerPawn : public APawn, public IInteractableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASchemePlayerPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnBeginFocus_Implementation(APawn* Interactor) override;
	virtual void OnEndFocus_Implementation(APawn* Interactor) override;
	virtual void OnInteract_Implementation(APawn* Interactor) override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FORCEINLINE USceneComponent* GetCameraRootComp() const { return CameraRootComp; }
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	class UInteractionComponent* InteractionComp;
	
	UCameraComponent* CameraComp;
	USceneComponent* CameraRootComp;
	ASchemePlayerController* PlayerController;

public:
	UPROPERTY(ReplicatedUsing = OnRep_CameraRotation)
	FRotator CameraRotation;

	/**
	 * Function called when the replicated property `CameraRotation` is updated on remote clients.
	 * It ensures that the camera's rotation is synchronized correctly on non-locally controlled clients.
	 *
	 * Functionality:
	 * - Updates the `CameraRootComp` component's relative rotation to match the new value of `CameraRotation`.
	 * - Logs a message indicating the updated camera rotation for debugging purposes.
	 *
	 * Requirements:
	 * - This function is executed on remote clients when the `CameraRotation` value is replicated.
	 * - The owning pawn must not be locally controlled for this function to take effect.
	 */
	UFUNCTION()
	void OnRep_CameraRotation();

};

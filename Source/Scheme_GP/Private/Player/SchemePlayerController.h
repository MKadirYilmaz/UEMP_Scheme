// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SchemePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ASchemePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ASchemePlayerController();
	
protected:
	virtual void BeginPlay() override;
	
public:
	// Called on a client, runs on the server
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Gold System")
	void SendGoldIncomeRequestToServer(int32 Amount);
	// Called on a client, runs on the server
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Gold System")
	void SendGoldOutcomeRequestToServer(int32 Amount);
	
	// Called in clients, works in server
	UFUNCTION(Server, Reliable, Category = "Interaction")
	void ServerRequestInteract(AActor* InteractActor, APawn* Interactor);

	UFUNCTION(Client, Reliable, Category = "Interaction")
	void ClientInteractNotify(AActor* InteractActor, APawn* Interactor);
private:
	UFUNCTION(BlueprintCallable)
	void HandleClampedRotation(float MouseInputYaw, float MouseInputPitch);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Movement Adjustments")
	float MaxYawLimit = 45.f;
	UPROPERTY(EditDefaultsOnly, Category = "Movement Adjustments")
	float MaxPitchLimit = 30.f;

	float YawRotationDelta;
	float PitchRotationDelta;

	/**
	 * Indicates whether camera rotation functionality is enabled.
	 *
	 * This property acts as a toggle to enable or disable the player's ability to rotate
	 * the camera. When set to true, camera rotation is allowed, and when false, any input
	 * related to camera rotation is ignored. It is primarily used to control camera behavior
	 * during gameplay scenarios where rotation should be restricted.
	 */
	bool bCameraRotationEnabled = true;

public:
	/**
	 * Enables or disables the camera rotation functionality.
	 *
	 * This method is used to toggle the player's ability to rotate the camera during gameplay.
	 * When enabled, the camera can respond to player input for rotation. When disabled, any
	 * camera rotation input will be ignored. The toggle can be useful in scenarios where the
	 * camera behavior needs to be restricted or locked for specific gameplay situations.
	 *
	 * @param bEnabled A boolean value indicating whether camera rotation should be enabled (true) or disabled (false).
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera Rotation System")
	FORCEINLINE void SetCameraRotationEnabled(bool bEnabled) {bCameraRotationEnabled = bEnabled;}
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SchemePlayerController.generated.h"

class ASchemeGameState;
class ASchemePlayerState;
class ASchemeGameMode;
class UActionDataAsset;
class UCardDataAsset;
class ACardTable;
class ACardActor;

struct FNotificationPacket;
enum class ECardRole : uint8;

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
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable, Category = "Game System")
	ASchemeGameMode* TryGetGameMode() const;
	UFUNCTION(BlueprintCallable, Category = "Game System")
	ASchemeGameState* TryGetGameState() const;
	
	// Called in server, works in clients
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Notification System")
	void Client_ReceiveNotification(const FNotificationPacket& Notification);
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Game System")
	void ExecuteAction(UActionDataAsset* ActionData, ASchemePlayerState* TargetState);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Game System")
	void StartGame();
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Game System")
	void Server_SendChallengeRequest();
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Game System")
	void Server_SendBlockRequest();
	
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Game System")
	void EndTurn();
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Gold System")
	void SendChangeGoldRequest(int32 Amount);
	
	UFUNCTION(Server, Reliable, Category = "Interaction")
	void Server_RequestInteract(AActor* InteractActor, APawn* Interactor);
	UFUNCTION(Client, Reliable, Category = "Interaction")
	void Client_InteractNotify(AActor* InteractActor, APawn* Interactor);
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Card System")
	void Server_AddCardToHand(ACardActor* NewCard);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Card System")
	void Server_RemoveCardFromHand(UCardDataAsset* CardToRemove);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Card System")
	void Server_RemoveRandomCardFromHand();
	
	UFUNCTION(BlueprintPure, Category = "Card System")
	bool HasCardInHand(const UCardDataAsset* CardToCheck) const;
	UFUNCTION(BlueprintPure, Category = "Card System")
	bool HasAnyCardInHand() const;
	UFUNCTION(BlueprintPure, Category = "Card System")
	ACardActor* GetCardFromHand(const ECardRole RoleToGet) const;
	
	// Handle UI when game starts
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Game System")
	void OnReceiveStartGameNotification();
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Game System")
	void OnReceiveChallengeNotification(const FText& Message);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Game System")
	void OnReceiveBlockNotification(const FText& Message);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Game System")
	void OnReceiveTimeoutNotification(const FText& Message);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Game System")
	void OnReceiveGeneralNotification(const FText& Message);
	
	UFUNCTION(BlueprintCallable)
	void PrintHoldingCards() const;
	
	UFUNCTION()
	void OnRep_HoldingCards() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Card System")
	int32 GetFirstEmptyCardHoldingPointIndex() const;
	FTransform GetCardHoldingPoint(int32 Index) const;
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Game System")
	void ResetState();
private:
	/**
	 * Handles camera rotation based on mouse input while enforcing yaw and pitch rotation limits.
	 *
	 * This function processes rotation inputs (yaw and pitch) from the user and applies them
	 * to the camera's local rotation. It ensures that the rotations are clamped within the defined
	 * maximum yaw and pitch limits (`MaxYawLimit` and `MaxPitchLimit` respectively). The function
	 * adjusts the rotation incrementally based on the input while preventing the camera from
	 * exceeding the allowed range.
	 *
	 * If the camera rotation system (`bCameraRotationEnabled`) is disabled, or if the camera's
	 * root component is not initialized, the function exits without performing any operations.
	 *
	 * Once the rotation is updated, it propagates the current yaw and pitch values to the server
	 * using the `HandleRotationInServer` method for synchronization in multiplayer scenarios.
	 *
	 * @param MouseInputYaw The input delta value for yaw rotation, typically derived from horizontal mouse movement.
	 * @param MouseInputPitch The input delta value for pitch rotation, typically derived from vertical mouse movement.
	 */
	UFUNCTION(BlueprintCallable)
	void HandleClampedRotation(float MouseInputYaw, float MouseInputPitch);
	/**
	 * Synchronizes the camera's yaw and pitch rotation values with the server.
	 *
	 * This function is called on the client-side to transmit the current yaw and pitch values
	 * to the server for synchronization. It ensures that the camera's rotation state remains
	 * consistent across the server and all connected clients in multiplayer scenarios.
	 *
	 * This function is marked as unreliable, indicating that the updates may be sent less frequently,
	 * which is appropriate for scenarios where precision is less critical and performance is a priority.
	 *
	 * @param Yaw The current yaw rotation value of the camera to be synchronized with the server.
	 * @param Pitch The current pitch rotation value of the camera to be synchronized with the server.
	 */
	UFUNCTION(Server, Unreliable)
	void HandleRotationInServer(float Yaw, float Pitch);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Movement Adjustments")
	float MaxYawLimit = 45.f;
	UPROPERTY(EditDefaultsOnly, Category = "Movement Adjustments")
	float MaxPitchLimit = 30.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_HoldingCards, VisibleAnywhere, Category = "Card System")
	TArray<ACardActor*> HoldingCards;
	
	UPROPERTY(EditDefaultsOnly, Category = "Card System")
    TSubclassOf<ACardTable> CardTableClass;
    UPROPERTY()
    ACardTable* CardTable;
	
	UPROPERTY()
	USceneComponent* CameraRootComp;
	
	float CurrentYawDelta;
	float CurrentPitchDelta;

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

	FORCEINLINE void SetCameraRootComponent(USceneComponent* NewCameraRoot) { CameraRootComp = NewCameraRoot; }
};

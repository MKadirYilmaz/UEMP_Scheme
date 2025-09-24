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

	
};

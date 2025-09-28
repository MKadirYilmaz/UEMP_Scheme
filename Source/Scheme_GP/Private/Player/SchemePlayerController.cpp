// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SchemePlayerController.h"

#include "Framework/SchemeGameMode.h"
#include "Kismet/KismetMathLibrary.h"

void ASchemePlayerController::RequestGoldIncome(int32 Amount)
{
	ASchemeGameMode* GameMode = GetWorld()->GetAuthGameMode<ASchemeGameMode>();
	if (GameMode)
	{
		GameMode->TryProcessGoldIncome(this, Amount);
	}
}

void ASchemePlayerController::RequestGoldOutcome(int32 Amount)
{
	ASchemeGameMode* GameMode = GetWorld()->GetAuthGameMode<ASchemeGameMode>();
	if (GameMode)
	{
		GameMode->TryProcessGoldOutcome(this, Amount);
	}
}

void ASchemePlayerController::HandleClampedRotation(float MouseInputYaw, float MouseInputPitch)
{
	// Yaw rotation calculation
	float CurrDelta = MouseInputYaw + YawRotationDelta;
	// Check if we reached to yaw limit
	if (MaxYawLimit > UKismetMathLibrary::Abs(CurrDelta))
	{
		// We didn't reach to limit
		YawRotationDelta = CurrDelta;
		// Rotate with MouseInputYaw value
		AddYawInput(MouseInputYaw);
	}
	else
	{
		float Delta = UKismetMathLibrary::Abs(CurrDelta) - MaxYawLimit;
		float NewRotationChange = (MouseInputYaw > 0) ? MouseInputYaw - Delta : MouseInputYaw + Delta;

		YawRotationDelta += NewRotationChange;
		// Rotate with NewRotationChange value
		AddYawInput(NewRotationChange);
	}
	// Pitch rotation calculation
	CurrDelta = MouseInputPitch + PitchRotationDelta;
	// Check if we reached to pitch limit
	if (MaxPitchLimit > UKismetMathLibrary::Abs(CurrDelta))
	{
		// We didn't reach to limit
		PitchRotationDelta = CurrDelta;
		// Rotate with MouseInputPitch value
		AddPitchInput(-MouseInputPitch);
	}
	else
	{
		float Delta = UKismetMathLibrary::Abs(CurrDelta) - MaxPitchLimit;
		float NewRotationChange = (MouseInputPitch > 0) ? MouseInputPitch - Delta : MouseInputPitch + Delta;

		PitchRotationDelta += NewRotationChange;
		// Rotate with NewRotationChange value
		AddPitchInput(-NewRotationChange);
	}
	
}

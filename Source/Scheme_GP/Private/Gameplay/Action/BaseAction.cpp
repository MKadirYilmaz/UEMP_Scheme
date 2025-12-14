// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Action/BaseAction.h"
#include "Framework/SchemeGameState.h"
#include "Player/SchemePlayerController.h"

void UBaseAction::ExecuteAction_Implementation(ASchemePlayerController* Instigator, ASchemePlayerController* Target)
{
	UE_LOG(LogTemp, Warning, TEXT("Action Executed but has no logic!"));
}

ASchemeGameState* UBaseAction::GetSchemeGameState() const
{
	return Cast<ASchemeGameState>(GetWorld()->GetGameState());
}

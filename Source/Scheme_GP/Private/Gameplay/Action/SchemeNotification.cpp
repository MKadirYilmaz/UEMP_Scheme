// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Action/SchemeNotification.h"

void USchemeNotification::HandleNotification_Implementation(ASchemePlayerController* Controller, const FText& NotMessage, UDataAsset* NotAsset, const TArray<UObject*>& NotObjects)
{
	UE_LOG(LogTemp, Warning, TEXT("Notification Received: %s"), *NotMessage.ToString());
}


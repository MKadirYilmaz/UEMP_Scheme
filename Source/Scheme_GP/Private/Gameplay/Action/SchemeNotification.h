// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SchemeNotification.generated.h"

class ASchemePlayerController;
class USchemeNotification;
USTRUCT(BlueprintType)
struct FNotificationPacket
{
	GENERATED_BODY()
public:
	static FNotificationPacket CreateNotificationPacket(const TSubclassOf<USchemeNotification>& Type, const FText& Message, UDataAsset* Asset = nullptr, const TArray<UObject*>& Objects = TArray<UObject*>())
	{
		FNotificationPacket Packet;
		Packet.NotificationType = Type;
		Packet.NotificationMessage = Message;
		Packet.NotificationAsset = Asset;
		Packet.NotificationObjects = Objects;
		return Packet;
	}
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Notification")
	TSubclassOf<USchemeNotification> NotificationType;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Notification")
	FText NotificationMessage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Notification")
	UDataAsset* NotificationAsset = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Notification")
	TArray<UObject*> NotificationObjects;
};

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class USchemeNotification : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Notification")
	void HandleNotification(ASchemePlayerController* Controller, const FText& NotMessage, UDataAsset* NotAsset, const TArray<UObject*>& NotObjects);
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Actors/CardActor.h"

#include "Net/UnrealNetwork.h"
#include "Gameplay/Data/CardDataAsset.h"

ACardActor::ACardActor()
{
	CardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Card Mesh"));
	CardMesh->SetupAttachment(RootComponent);
	
	bReplicates = true;
}

void ACardActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ACardActor, CardData, COND_OwnerOnly);
}

void ACardActor::SetCardData(UCardDataAsset* NewData)
{
	CardData = NewData;
	if (!CardData)
	{
		UE_LOG(LogTemp, Error, TEXT("%s: Card Data is NULL!"), (HasAuthority()) ? TEXT("Server") : TEXT("Client"));
	}
}

void ACardActor::OnRep_CardData()
{
	if (!CardData)
	{
		UE_LOG(LogTemp, Error, TEXT("%s: Card Data is NULL!"), (HasAuthority()) ? TEXT("Server") : TEXT("Client"));
		return;
	}
	CardMesh->SetMaterial(0, CardData->GetCardMaterial());
}

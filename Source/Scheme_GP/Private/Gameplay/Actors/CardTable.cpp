// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Actors/CardTable.h"

// Sets default values
ACardTable::ACardTable()
{
	TableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Table Mesh"));

	RootComponent = TableMesh;
	
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACardTable::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACardTable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACardTable::CalculateCardLocationPairs()
{
	FVector TableBottomMiddle = TableMesh->GetComponentLocation();
	FVector TableTopMiddle = TableBottomMiddle + TableMesh->GetUpVector() * 85.f;

	DrawDebugLine(GetWorld(), TableBottomMiddle, TableTopMiddle, FColor::Red, false, 0.f, 0, 0.5f);
	
	TArray<FTransform> CardTransforms;
	for (int32 i = 0; i < 16; i++)
	{
		FVector TableCardLocation = TableTopMiddle + TableMesh->GetForwardVector().RotateAngleAxis(22.5f * i + CardPointsRotationOffset, FVector::UpVector) * 50.f;
		DrawDebugLine(GetWorld(), TableTopMiddle, TableCardLocation, FColor::Blue, false, 0.f, 0, 0.5f);
		
		CardTransforms.Add(FTransform((TableTopMiddle - TableCardLocation).Rotation(), TableCardLocation, FVector::OneVector));
	}
	// Clear the array
	CardPointsStructs.Empty();
	// First Player Points
	CardPointsStructs.Add(FCardLocationStruct{ TArray<FTransform> {CardTransforms[0], CardTransforms[15]} });
	// Second Player Points ....
	CardPointsStructs.Add(FCardLocationStruct{ TArray<FTransform> {CardTransforms[7], CardTransforms[8]} });
	CardPointsStructs.Add(FCardLocationStruct{ TArray<FTransform> {CardTransforms[11], CardTransforms[12]} });
	CardPointsStructs.Add(FCardLocationStruct{ TArray<FTransform> {CardTransforms[3], CardTransforms[4]} });
	CardPointsStructs.Add(FCardLocationStruct{ TArray<FTransform> {CardTransforms[1], CardTransforms[2]} });
	CardPointsStructs.Add(FCardLocationStruct{ TArray<FTransform> {CardTransforms[9], CardTransforms[10]} });
	CardPointsStructs.Add(FCardLocationStruct{ TArray<FTransform> {CardTransforms[13], CardTransforms[14]} });
	CardPointsStructs.Add(FCardLocationStruct{ TArray<FTransform> {CardTransforms[5], CardTransforms[6]} });

	for (FCardLocationStruct& CardLocationStruct : CardPointsStructs)
	{
		for (FTransform& CardTransform : CardLocationStruct.CardTransforms)
		{
			DrawDebugSphere(GetWorld(), CardTransform.GetLocation(), 10.f, 10, FColor::Green, false, 0.f, 0, 0.5f);
		}
	}
	
}


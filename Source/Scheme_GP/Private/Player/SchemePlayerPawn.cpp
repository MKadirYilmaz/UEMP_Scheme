// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SchemePlayerPawn.h"

#include "InteractionComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
ASchemePlayerPawn::ASchemePlayerPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	InteractionComp = CreateDefaultSubobject<UInteractionComponent>("Interaction Component");
	
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
}

// Called when the game starts or when spawned
void ASchemePlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	CameraComp = GetComponentByClass<UCameraComponent>();
	InteractionComp->SetLineTraceStartComp(CameraComp);
	
}

// Called every frame
void ASchemePlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASchemePlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


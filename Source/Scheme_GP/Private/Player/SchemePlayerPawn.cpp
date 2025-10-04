// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SchemePlayerPawn.h"

#include "InteractionComponent.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/SchemePlayerController.h"

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

	PlayerController = Cast<ASchemePlayerController>(GetController());
	CameraComp = GetComponentByClass<UCameraComponent>();
	
	if (CameraComp)
	{
		CameraRootComp = CameraComp->GetAttachParent();
		
		if (IsLocallyControlled())
		{
			if (UStaticMeshComponent* HeadMesh = Cast<UStaticMeshComponent>(CameraRootComp))
			{
				HeadMesh->SetOwnerNoSee(true);
			}
			
			if (InteractionComp)
			{
				InteractionComp->SetLineTraceStartComp(CameraComp);
			}
		}
		
		
		if (PlayerController)
		{
			PlayerController->SetCameraRootComponent(CameraRootComp);
		}
		
		UE_LOG(LogTemp, Display, TEXT("Pawn %s: CameraRootComp INITIALIZED (IsLocal: %s)"), 
			*GetName(), IsLocallyControlled() ? TEXT("YES") : TEXT("NO"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Pawn %s: CameraComp is NULL!"), *GetName());
	}
	
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

void ASchemePlayerPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASchemePlayerPawn, CameraRotation);
}

void ASchemePlayerPawn::OnRep_CameraRotation()
{
	// Remote client
	if (CameraRootComp && !IsLocallyControlled())
	{
		CameraRootComp->SetRelativeRotation(CameraRotation);
		UE_LOG(LogTemp, Display, TEXT("REMOTE Pawn %s: Rotation Updated to %s"), 
			*GetName(), *CameraRotation.ToString());
	}
}


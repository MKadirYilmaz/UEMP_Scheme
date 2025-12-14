// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BaseAction.generated.h"


class ASchemePlayerController;
class ASchemeGameState;
/**
 * 
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class UBaseAction : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ExecuteAction(ASchemePlayerController* Instigator, ASchemePlayerController* Target);
	
	virtual void ExecuteAction_Implementation(ASchemePlayerController* Instigator, ASchemePlayerController* Target);

protected:
	ASchemeGameState* GetSchemeGameState() const;
};

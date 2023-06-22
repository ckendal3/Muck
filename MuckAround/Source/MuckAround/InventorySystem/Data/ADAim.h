// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MuckAround/ActionSystem/Data/ActionDefinition.h"
#include "ADAim.generated.h"

/**
 * 
 */
UCLASS()
class MUCKAROUND_API UADAim : public UActionDefinition
{
	GENERATED_BODY()

public:

	UADAim();

	
	UPROPERTY(VisibleInstanceOnly)
	float StoredMaxWalkSpeed;

	virtual void BeginAction_Implementation() override;
	virtual void EndAction_Implementation() override;
};

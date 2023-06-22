// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ActionConfig.generated.h"

/**
 * 
 */
UCLASS()
class MUCKAROUND_API UActionConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	// Identifier used for determining the cast type.
	UPROPERTY(EditAnywhere)
	FGameplayTag ConfigIdentifier;
};

/**
 * 
 */
UCLASS()
class MUCKAROUND_API UDamageConfig : public UActionConfig
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	float DamageAmount = 10.f;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UDamageType> DamageType;
};
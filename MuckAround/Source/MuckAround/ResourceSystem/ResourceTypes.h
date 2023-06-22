// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "ResourceTags.h"
#include "ResourceTypes.generated.h"

USTRUCT(BlueprintType)
struct FResourceStruct
{
	GENERATED_BODY()

	FResourceStruct()
	{
		Identifier = TAG_RESOURCE_UNASSIGNED;
		Value		= 0.f;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag Identifier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Value;
};

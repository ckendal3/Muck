// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SurvivalTags.h"
#include "SurvivalTypes.generated.h"

USTRUCT(BlueprintType)
struct FSurvivalStruct
{
	GENERATED_BODY()

	FSurvivalStruct()
	{
		Identifier	= TAG_SURVIVAL_UNASSIGNED;
		Value		= 0.f;
	}

	FSurvivalStruct(FGameplayTag InTag, float InValue)
	{
		Identifier	= InTag;
		Value		= InValue;
	}
	
	UPROPERTY(EditAnywhere)
	FGameplayTag Identifier;

	UPROPERTY(EditAnywhere)
	float Value;
};

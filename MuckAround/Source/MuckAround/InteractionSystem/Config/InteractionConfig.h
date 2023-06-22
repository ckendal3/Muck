// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MuckAround/ActionSystem/Data/ActionConfig.h"
#include "InteractionConfig.generated.h"

/**
 * 
 */
UCLASS()
class MUCKAROUND_API UInteractionConfig : public UActionConfig
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float OverlapCheckSize = 150.f;
};
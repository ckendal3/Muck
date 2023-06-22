// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "TaggableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable, BlueprintType)
class UTaggableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Generic interface for setting a gameplay tag on an object.
 */
class MUCKAROUND_API ITaggableInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetTag(FGameplayTag InTag);
	
};

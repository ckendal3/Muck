// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "InteractionInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MUCKAROUND_API IInteractionInterface
{
	GENERATED_BODY()

public:
	
	/**
	 * @brief Set the interaction state of an object
	 * @param InTag The Interaction Tag Type
	 * @param HeldTime How long has the interaction been held
	 * @param Instigator What object called this
	 * @return If the interaction has finished
	 */
	UFUNCTION(BlueprintNativeEvent)
		bool SetInteractionState(FGameplayTag InTag, float HeldTime = 0.f, UObject* Instigator = nullptr);

	UFUNCTION(BlueprintNativeEvent)
		void GetDesiredInteraction(FGameplayTag& OutTag, float& OutTime);
};

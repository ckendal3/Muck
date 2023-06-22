// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "AnimationDefinition.generated.h"

class UAnimSequenceBase;

USTRUCT(BlueprintType)
struct FAnimationSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UAnimSequenceBase*> Animations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DefaultOverrideSlotName = FName("");
	
	UAnimSequenceBase* GetAnimation(int Index = 0) const;
	UAnimSequenceBase* GetAnimationRandom() const;
};

/**
 * Animation definitions based on gameplay tags.
 */
UCLASS(BlueprintType, Blueprintable)
class MUCKAROUND_API UAnimationDefinition : public UDataAsset
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, FAnimationSet> AnimationMap;

public:
	UFUNCTION(BlueprintPure)
	UAnimSequenceBase* GetAnimation(FGameplayTag InTag, int Index = 0);

	UFUNCTION(BlueprintPure)
	UAnimSequenceBase* GetAnimationRandom(FGameplayTag InTag);

	UFUNCTION(BlueprintPure)
	FAnimationSet GetAnimationSet(FGameplayTag InTag);	
};

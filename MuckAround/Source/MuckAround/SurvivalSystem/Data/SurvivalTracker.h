// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MuckAround/ActionSystem/Data/ActionDefinition.h"
#include "MuckAround/SurvivalSystem/SurvivalTypes.h"
#include "SurvivalTracker.generated.h"

/**
 * Base class for tracking survival traits.
 */
UCLASS()
class MUCKAROUND_API USurvivalTracker : public UActionDefinition
{
	GENERATED_BODY()

public:

	USurvivalTracker();
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_SurvivalTraits)
	TArray<FSurvivalStruct> SurvivalTraits;

	// Determines how fast a trait changes per a tick.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FSurvivalStruct> TraitsDeltas;

	UFUNCTION()
	void OnRep_SurvivalTraits();

	UFUNCTION(BlueprintCallable)
	void AdjustSurvivalTrait(FGameplayTag InSurvivalTag, float InValue);

	UFUNCTION(BlueprintNativeEvent)
	void OnSurvivalTraitChanged();

	void ApplyTraitDelta();

	virtual void InitializeAction(UActionComponent* Owner) override;
	virtual void TickAction_Implementation() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MuckAround/ActionSystem/Data/ActionDefinition.h"
#include "MuckAround/ResourceSystem/ResourceTypes.h"
#include "UObject/Object.h"
#include "ResourceTracker.generated.h"

/**
 * Base class for tracking resources.
 */
UCLASS()
class MUCKAROUND_API UResourceTracker : public UActionDefinition
{
	GENERATED_BODY()

public:

	UResourceTracker();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_Resources)
	TArray<FResourceStruct> Resources;

	UFUNCTION()
	void OnRep_Resources();

	UFUNCTION(BlueprintCallable)
	void AddResource(FGameplayTag InResourceTag, float InValue);

	UFUNCTION(BlueprintNativeEvent)
	void OnResourceChanged(FGameplayTag ResourceTag, float NewValue);

	UFUNCTION(BlueprintNativeEvent, meta = (DeprecatedFunction))
	void OnResourcesChanged();

	virtual void InitializeAction(UActionComponent* Owner) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

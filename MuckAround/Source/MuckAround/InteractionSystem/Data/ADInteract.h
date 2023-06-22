// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MuckAround/ActionSystem/Data/ActionDefinition.h"
#include "ADInteract.generated.h"

/**
 * 
 */
UCLASS()
class MUCKAROUND_API UADInteract : public UActionDefinition
{
	GENERATED_BODY()

public:

	UADInteract();
	
	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<AActor> ValidActor;

	UPROPERTY(VisibleInstanceOnly)
	float HoldTime = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    float OverlapCheckSize = 300.f;

	UPROPERTY()
	FTimerHandle   HeldHandle;
	FTimerDelegate HeldDelegate;
	
protected:

	virtual void SetupFromConfig(UActionConfig* InConfig) override;

	virtual void InitializeAction(UActionComponent* Owner) override;
	virtual void TickAction_Implementation() override;
	virtual void CustomStateAction_Implementation() override;
	
	UFUNCTION(BlueprintNativeEvent)
	void GetOwnerViewAndLocation(FVector& OutLocation, FRotator& OutDirection);

	float DotFromOwnerViewAndLocation(AActor* Actor, const FVector& LookFromLocation, const FRotator& Direction);

	UFUNCTION(BlueprintCallable)
	void SetTargetActor(AActor* TargetActor);

	UFUNCTION(BlueprintCallable)
	void ClearTargetActor();

	void AttemptInteraction();

	UFUNCTION()
	void InteractionHeld();
};

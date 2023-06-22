// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MuckAround/ActionSystem/Interfaces/TaggableInterface.h"
#include "StatefulActor.generated.h"

UCLASS()
class MUCKAROUND_API AStatefulActor : public AActor,  public ITaggableInterface
{
	GENERATED_BODY()

public:
	
	AStatefulActor();

	virtual void SetTag_Implementation(FGameplayTag InTag) override;

	FGameplayTag GetCurrentState() const { return CurrentState; }
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tag)
	FGameplayTag DefaultState;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentState, Category = Tag)
	FGameplayTag CurrentState;
	
	UFUNCTION(BlueprintNativeEvent)
	void ReceiveNewState(FGameplayTag PreviousTag, FGameplayTag NewTag);
	
	UFUNCTION()
	void OnRep_CurrentState(FGameplayTag PreviousState);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, CallInEditor)
	void EDITOR_UpdateTagState(FGameplayTag NewTag);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

#if WITH_EDITOR

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

#endif
};

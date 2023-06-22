// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "../ActionTypes.h"
#include "Components/ActorComponent.h"
#include "ActionComponent.generated.h"

class UActionDefinition;

// TODO: GET ALL ACTIONS WITH TAG AND STATE
// TODO: REMOVE ALL ACTIONS WITH TAG AND STATE
// TODO: Action state need to be defined as tags - no enums
// TODO: Action limit type need to be defined as tags - no enums

/*
 * The base class for the action component which is a management component for actions via ownership.
 * This component can be used on any actor type but it is primarily used for characters and weapons.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UActionComponent();

	// The actions that belong to this component.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated)
		TArray<UActionDefinition*> Actions;

	// The actions that will automatically be created and added.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<TSubclassOf<UActionDefinition>> DefaultActions;

	// The tags that are currently active on this component.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
		FGameplayTagContainer ActiveTags;

	// Create an action and add it to the Actions array.
	UFUNCTION(BlueprintCallable)
		virtual void CreateAndAddAction(TSubclassOf<UActionDefinition> InActionClass);

	UFUNCTION(BlueprintPure)
		bool CanAddAction() const;
	
	// Start an action by finding an exact match via the tag.
	UFUNCTION(BlueprintCallable)
		virtual void StartActionByTag(FGameplayTag InTag);
	
	// Stop an action by finding an exact match via the tag.
	UFUNCTION(BlueprintCallable)
		virtual void StopActionByTag(FGameplayTag InTag);

	void StartAction(UActionDefinition* Action);

	void StopAction(UActionDefinition* Action);

	UFUNCTION(BlueprintCallable)
		void SetActionStateByTag(FGameplayTag ActionTag, FGameplayTag StateTag);

	void SetActionState(UActionDefinition* Action, FGameplayTag StateTag);
	
	// Stop all running actions.
	UFUNCTION(BlueprintCallable)
		virtual void StopAllActions();

	// Check if this action can be started? NOTE: Internally calls C++ CanStartAction()
	UFUNCTION(BlueprintPure, DisplayName = "CanStartActionByTag")
		bool CanStartAction_Tag(FGameplayTag ActionToFind) const;

	// Determine if this action can be started. This should be overridden in child classes.
	virtual bool CanStartAction(UActionDefinition* InAction) const;

	// Determine if this action is already running.
	UFUNCTION(BlueprintPure)
		bool IsActionRunning(FGameplayTag InTag) const;

	// Get the action object by the tag identifier.
	UFUNCTION(BlueprintPure)
		UActionDefinition* GetAction(FGameplayTag ActionToFind) const;

	// Get all actions that are currently running.
	UFUNCTION(BlueprintPure)
		TArray<UActionDefinition*> GetActiveActions() const;
	
	// Get all actions that are currently running.
	UFUNCTION(BlueprintPure)
		void GetAllActionsOfTags(FGameplayTagContainer Tags, TArray<UActionDefinition*>& OutActions) const;

	// Get all actions that are currently running.
	UFUNCTION(BlueprintCallable)
		void RemoveAllActionsOfTags(FGameplayTagContainer Tags);

	void RemoveActionByTag(FGameplayTag InTag);
	void RemoveAction(UActionDefinition* Action);

protected:

	// Initializes the 'DefaultActions' array of classes
	virtual void InitializeDefaultActions();

	// Adds an existing action to the 'Actions' array for usage with the initial state that should be set.
	// NOTE: Setting the action to Start or Performing will active it. Setting it to End will not activate it.
	virtual void AddAction(UActionDefinition* InAction, EActionState InState = EActionState::AS_End);
	
	// ENGINE FUNCTIONS //
	
	virtual void OnRegister() override;
	virtual void DestroyComponent(bool bPromoteChildren) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
};

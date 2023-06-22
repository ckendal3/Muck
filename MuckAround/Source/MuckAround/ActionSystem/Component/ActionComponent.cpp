// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionComponent.h"
#include "../Data/ActionDefinition.h"
#include "Engine/ActorChannel.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

UActionComponent::UActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	SetIsReplicatedByDefault(true);
}

void UActionComponent::CreateAndAddAction(TSubclassOf<UActionDefinition> InActionClass)
{
	ensureMsgf(IsValid(InActionClass), TEXT("UActionComponent::CreateAndAddAction received an invalid ActionDefinition!"));

	if(InActionClass && CanAddAction())
	{
		// We use our actor as the owner because of replication. Using AActor rather than UActorComponent
		// should prevent weird issues - per online discussion advice
		UActionDefinition* NewActionObject = NewObject<UActionDefinition>(GetOwner(), InActionClass);

		AddAction(NewActionObject);
	}
}

bool UActionComponent::CanAddAction() const
{
	return true;
}

void UActionComponent::StartActionByTag(FGameplayTag InTag)
{
	UActionDefinition* FoundAction = GetAction(InTag);
	StartAction(FoundAction);
}

void UActionComponent::StartAction(UActionDefinition* Action)
{
	ensure(IsValid(Action));
	if(IsValid(Action) && CanStartAction(Action))
	{
		Action->SetTag_Implementation(TAG_ACTION_START);
	}
}

void UActionComponent::StopActionByTag(FGameplayTag InTag)
{
	UActionDefinition* FoundAction = GetAction(InTag);
	StopAction(FoundAction);
}

void UActionComponent::StopAction(UActionDefinition* Action)
{
	if(IsValid(Action) && Action->IsActive())
	{
		Action->SetTag_Implementation(TAG_ACTION_END);
	}
}

void UActionComponent::SetActionStateByTag(FGameplayTag ActionTag, FGameplayTag StateTag)
{
	UActionDefinition* FoundAction = GetAction(ActionTag);
	SetActionState(FoundAction, StateTag);
}

void UActionComponent::SetActionState(UActionDefinition* Action, FGameplayTag StateTag)
{
	if(IsValid(Action))
	{
		Action->SetTag_Implementation(StateTag);
	}
}

void UActionComponent::StopAllActions()
{
	for(UActionDefinition* Action : Actions)
	{
		if(Action && Action->IsActive())
		{
			Action->SetTag_Implementation(TAG_ACTION_END);
		}
	}
}

void UActionComponent::InitializeDefaultActions()
{	
	for(const TSubclassOf<UActionDefinition> ActionClass : DefaultActions)
	{
		CreateAndAddAction(ActionClass);
	}
}

void UActionComponent::AddAction(UActionDefinition* InAction, EActionState InState)
{
	if(InAction && Actions.Contains(InAction) == false)
	{
		Actions.Add(InAction);

		InAction->InitializeAction(this);
		
		if(InState == EActionState::AS_Start)
		{
			InAction->SetTag_Implementation(TAG_ACTION_START);
		}
	}
}

void UActionComponent::RemoveActionByTag(FGameplayTag InTag)
{
	UActionDefinition* FoundAction = GetAction(InTag);
	RemoveAction(FoundAction);
}

void UActionComponent::RemoveAction(UActionDefinition* Action)
{
	if(IsValid(Action) && Actions.Contains(Action))
	{
		StopAction(Action);
		
		Actions.Remove(Action);
	}
}

bool UActionComponent::CanStartAction_Tag(FGameplayTag ActionToFind) const
{
	UActionDefinition* FoundAction = GetAction(ActionToFind);
	if(FoundAction)
	{
		return CanStartAction(FoundAction);
	}

	return false;
}

bool UActionComponent::CanStartAction(UActionDefinition* InAction) const
{
	return InAction->CanStart() && !ActiveTags.HasAnyExact(InAction->BlockingTags);
}

bool UActionComponent::IsActionRunning(FGameplayTag InTag) const
{
	const UActionDefinition* FoundAction = GetAction(InTag);
	if(FoundAction)
	{
		return FoundAction->IsActive();
	}
	
	return false;
}

UActionDefinition* UActionComponent::GetAction(FGameplayTag ActionToFind) const
{
	if(Actions.Num() > 0)
	{
		for(UActionDefinition* Action : Actions)
		{
			if(Action && Action->Identifier.MatchesTagExact(ActionToFind))
			{
				return Action;
			}
		}
	}

	return nullptr;
}

TArray<UActionDefinition*> UActionComponent::GetActiveActions() const
{
	TArray<UActionDefinition*> RunningActions;
	for(UActionDefinition* Action : Actions)
	{
		if(Action->IsActive())
		{
			RunningActions.Add(Action);
		}
	}

	return RunningActions;
}

void UActionComponent::GetAllActionsOfTags(FGameplayTagContainer Tags, TArray<UActionDefinition*>& OutActions) const
{
	for(UActionDefinition* Action : Actions)
	{
		ensure(IsValid(Action));
		if(IsValid(Action) && Action->SortingTags.HasAnyExact(Tags))
		{
			OutActions.Add(Action);
		}
	}
}

void UActionComponent::RemoveAllActionsOfTags(FGameplayTagContainer Tags)
{
	TArray<int> ActionIndicesToRemove;

	// Find all the index of items to remove
	int Index = 0;
	for(UActionDefinition* Action : Actions)
	{
		ensure(IsValid(Action));
		if(IsValid(Action) && Action->SortingTags.HasAnyExact(Tags))
		{
			ActionIndicesToRemove.Add(Index);
		}
		
		Index++;
	}

	// Reverse loop and remove the elements
	for(int i = ActionIndicesToRemove.Num() - 1; i > 0; i--)
	{
		const int IndexToRemove = ActionIndicesToRemove[i];

		ensure(IsValid(Actions[IndexToRemove]));
		if(IsValid(Actions[IndexToRemove]))
		{
			RemoveAction(Actions[IndexToRemove]);
		}
	}
}

void UActionComponent::OnRegister()
{
	Super::OnRegister();
	
	// Make sure we are something other than client
	if(GetWorld()->IsGameWorld() && GetWorld()->GetNetMode() < ENetMode::NM_Client)
	{
		InitializeDefaultActions();
	}
}

void UActionComponent::DestroyComponent(bool bPromoteChildren)
{
	StopAllActions();
	
	Super::DestroyComponent(bPromoteChildren);
}

void UActionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UActionComponent, Actions);
}

bool UActionComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bDirty = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	bDirty |= Channel->ReplicateSubobjectList(Actions, *Bunch, *RepFlags);
	return bDirty;
}
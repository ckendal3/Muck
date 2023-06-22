// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionDefinition.h"
#include "../Component/ActionComponent.h"
#include "Engine/NetDriver.h"
#include "MuckAround/ActionSystem/ActionTags.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

UActionDefinition::UActionDefinition()
{
	TimerDelegate.BindUObject(this, &UActionDefinition::Internal_TickAction);
}

void UActionDefinition::InitializeAction(UActionComponent* Owner)
{
	OwningComponent = Owner;
}

void UActionDefinition::SetupFromConfig(UActionConfig* InConfig) { }

void UActionDefinition::ResetCount()
{
	TickCount = 0;
}

bool UActionDefinition::IsLocallyOwned() const
{
	return GetOwningActor() && GetOwningActor()->HasLocalNetOwner();
}

void UActionDefinition::Internal_BeginAction()
{
	NetworkStateRequest(TAG_ACTION_START);

	CurrentStateTag = TAG_ACTION_START;

	UpdateActiveTags();

	BeginAction();

	if(OnActionEventListener.IsBound())
	{
		OnActionEventListener.Broadcast(this, TAG_ACTION_START);
	}
	
	switch(ActionEndType)
	{
	case EActionEndType::AET_Immediate:
		Internal_EndAction();
		break;
	case EActionEndType::AET_Timer:
		ResetCount();
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, TickRate, true, 0.f);
		break;
	case EActionEndType::AET_Manual:
		break;
	default: ;
	}
}

void UActionDefinition::Internal_TickAction()
{
	CurrentStateTag = TAG_ACTION_TICKING;
	
	TickCount++;
	
	TickAction();

	if(OnActionEventListener.IsBound())
	{
		OnActionEventListener.Broadcast(this, TAG_ACTION_TICKING);
	}
	
	if(LimitType == ELimiterType::LT_Limited && TickCount >= MaxTicks)
	{
		Internal_EndAction();
	}
}

void UActionDefinition::Internal_CustomStateAction(FGameplayTag CustomTag)
{
	NetworkStateRequest(CustomTag);
	
	CurrentStateTag = CustomTag;

	CustomStateAction();
	
	if(OnActionEventListener.IsBound())
	{
		OnActionEventListener.Broadcast(this, CustomTag);
	}
}

void UActionDefinition::Internal_EndAction()
{
	NetworkStateRequest(TAG_ACTION_END);
	
	CurrentStateTag = TAG_ACTION_END;
	
	UpdateActiveTags();
	
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

	EndAction();

	if(OnActionEventListener.IsBound())
	{
		OnActionEventListener.Broadcast(this, TAG_ACTION_END);
	}
}

void UActionDefinition::UpdateActiveTags()
{
	if(IsValid(OwningComponent))
	{
		if(IsActive())
		{
			OwningComponent->ActiveTags.AppendTags(OperatingTags);
		}
		else
		{
			OwningComponent->ActiveTags.RemoveTags(OperatingTags);
		}
	}
}

void UActionDefinition::BeginAction_Implementation() { }

void UActionDefinition::TickAction_Implementation() { }

void UActionDefinition::CustomStateAction_Implementation() { }

void UActionDefinition::EndAction_Implementation() { }

AActor* UActionDefinition::GetOwningActor() const
{
	return GetTypedOuter<AActor>();
}

void UActionDefinition::Destroy()
{
	if (!IsValid(this))
	{
		checkf(GetOwningActor()->HasAuthority() == true, TEXT("Destroy:: Object does not have authority to destroy itself!"));
			
		if(IsActive())
		{
			Internal_EndAction();
		}

		if(OwningComponent)
		{
			OwningComponent->RemoveAction(this);
		}
		
		MarkAsGarbage();
	}
}

bool UActionDefinition::CanStart_Implementation() const
{
	return IsActive() == false;
}

void UActionDefinition::Request_InternalEnd()
{
	if (IsActive())
	{
		Internal_EndAction();
	}
}

#pragma region Replication

void UActionDefinition::OnRep_CurrentStateTag(FGameplayTag PreviousTag)
{
	// Note: This will not get called if the client already has a matching action state.
	// If you want to use this functionality the client should not have control over this
	// variable's state. Only the server should change it.

	if(PreviousTag.MatchesTagExact(TAG_ACTION_END) && IsActive())
	{
		// If we weren't running and now are, start the action locally
		Internal_BeginAction();
	}
	else if(PreviousTag.MatchesTagExact(TAG_ACTION_END) == false && CurrentStateTag.MatchesTagExact(TAG_ACTION_END))
	{
		// If we were running and no longer should be, call end action
		Internal_EndAction();
	}
	else
	{
		Execute_SetTag(this, CurrentStateTag);
	}

	OnReceiveNewState(PreviousTag);
}

void UActionDefinition::OnRep_OwningComponent() { }

void UActionDefinition::NetworkStateRequest(FGameplayTag NewState)
{
	if(GetWorld()->IsGameWorld() == false)
	{
		return;
	}

	if(CurrentStateTag.MatchesTagExact(NewState) == false && GetOwningActor()->HasLocalNetOwner())
	{
		if(GetWorld()->GetNetMode() == NM_Client)
		{
			Server_R_SetActionState(NewState);
		}
		else if(GetWorld()->GetNetMode() < NM_Client && ActionEndType == EActionEndType::AET_Immediate)
		{
			NetMulticast_R_SetActionState(CurrentStateTag);
		}	
	}
}

// Design for networking bypass/workaround https://jambax.co.uk/replicating-uobjects/

void UActionDefinition::Server_R_SetActionState_Implementation(FGameplayTag NewState)
{
	// Force a networked multicast call if its immediate
	if(ActionEndType == EActionEndType::AET_Immediate)
	{
		NetMulticast_R_SetActionState(NewState);
	}
	else // else we just bypass the network side and do it locally via _Implementation
	{
		NetMulticast_R_SetActionState_Implementation(NewState);
	}
}

void UActionDefinition::NetMulticast_R_SetActionState_Implementation(FGameplayTag NewState)
{
	FGameplayTag OldTag = CurrentStateTag;

	CurrentStateTag = NewState;
	
	OnRep_CurrentStateTag(OldTag);
}

void UActionDefinition::SetTag_Implementation(FGameplayTag InTag)
{
	if(InTag.MatchesTagExact(TAG_ACTION_START) && IsActive() == false)
	{
		Internal_BeginAction();
	}
	else if(InTag.MatchesTagExact(TAG_ACTION_END) && IsActive())
	{
		Internal_EndAction();
	}
	else if(InTag.MatchesTagExact(TAG_ACTION_TICKING) && CurrentStateTag.MatchesTagExact(TAG_ACTION_TICKING) == false)
	{
		Internal_TickAction();
	}
	else if(!InTag.MatchesTagExact(TAG_ACTION_END) || !(InTag.MatchesTagExact(TAG_ACTION_START)))
	{
		Internal_CustomStateAction(InTag);
	}
}

bool UActionDefinition::IsSupportedForNetworking() const
{
	return true;
}

void UActionDefinition::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// Add any Blueprint properties
	// This is not required if you do not want the class to be "Blueprintable"
	if (const UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}

	DOREPLIFETIME(UActionDefinition, OwningComponent);
	DOREPLIFETIME(UActionDefinition, CurrentStateTag);
}

int32 UActionDefinition::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	check(GetOuter() != nullptr);
	return GetOwningActor() ? GetOwningActor()->GetFunctionCallspace(Function, Stack)  : FunctionCallspace::Local;
}
	
// Call "Remote" (aka, RPC) functions through the actors NetDriver
bool UActionDefinition::CallRemoteFunction(UFunction* Function, void* Params, struct FOutParmRec* OutParams, FFrame* Stack)
{
	check(!HasAnyFlags(RF_ClassDefaultObject));
	AActor* Owner = GetOwningActor();
	
	if (UNetDriver* NetDriver = Owner->GetNetDriver())
	{
		NetDriver->ProcessRemoteFunction(Owner, Function, Params, OutParams, Stack, this);
		return true;
	}
	
	return false;
}

#pragma endregion Replication

UWorld* UActionDefinition::GetWorld() const
{
	if ( HasAllFlags(RF_ClassDefaultObject) )
	{
		return nullptr;
	}
	
	UWorld* World = GetOwningActor() ? GetOwningActor()->GetWorld() : Cast<UWorld>(GetTypedOuter(UWorld::StaticClass())); 
	ensure(World);
	
	return World;
}

#if WITH_EDITOR
void UActionDefinition::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	const FName PropName = PropertyAboutToChange ? PropertyAboutToChange->GetFName() : FName();
	
	const bool bIsIdentifier  = PropName == GET_MEMBER_NAME_CHECKED(UActionDefinition, Identifier);

	if(bIsIdentifier)
	{
		SortingTags.RemoveTag(Identifier);
	}
}

void UActionDefinition::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropName = PropertyChangedEvent.GetPropertyName();

	const bool bIsCountChange = PropName == GET_MEMBER_NAME_CHECKED(UActionDefinition, MaxTicks)  || PropName == GET_MEMBER_NAME_CHECKED(UActionDefinition, TickRate);
	const bool bIsTimeRelated = PropName == GET_MEMBER_NAME_CHECKED(UActionDefinition, LimitType) || PropName == GET_MEMBER_NAME_CHECKED(UActionDefinition, ActionEndType);
	const bool bIsIdentifier  = PropName == GET_MEMBER_NAME_CHECKED(UActionDefinition, Identifier);
	
	if(bIsCountChange || bIsTimeRelated)
	{
		TotalTime = MaxTicks * TickRate;
	}
	else if(bIsIdentifier)
	{
		SortingTags.AddTag(Identifier);
	}
}
#endif
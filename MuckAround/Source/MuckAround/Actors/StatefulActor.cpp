// Fill out your copyright notice in the Description page of Project Settings.


#include "StatefulActor.h"
#include "Engine/World.h"
#include "MuckAround/ActionSystem/ActionTags.h"
#include "Net/UnrealNetwork.h"

AStatefulActor::AStatefulActor()
{
	PrimaryActorTick.bCanEverTick			= true;
	PrimaryActorTick.bStartWithTickEnabled	= false;

	bReplicates = true;
	
	DefaultState	= TAG_STATE_DEACTIVE;
	CurrentState	= TAG_STATE_TRANSIENT;
}

void AStatefulActor::ReceiveNewState_Implementation(FGameplayTag PreviousTag, FGameplayTag NewTag) { }

void AStatefulActor::SetTag_Implementation(FGameplayTag InTag)
{
	// Only server executes it
	if(GetWorld()->GetNetMode() < ENetMode::NM_Client)
	{
		const FGameplayTag PreviousState = CurrentState;

		// Make sure we don't already have this state
		if(PreviousState.MatchesTagExact(InTag) == false)
		{
			CurrentState = InTag;

			ReceiveNewState(PreviousState, CurrentState);
		}
	}
}

void AStatefulActor::OnRep_CurrentState(FGameplayTag PreviousState)
{
	ReceiveNewState(PreviousState, CurrentState);
}

void AStatefulActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStatefulActor, CurrentState);
}

void AStatefulActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	Execute_SetTag(this, DefaultState);
}

#if WITH_EDITOR

void AStatefulActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropName = PropertyChangedEvent.GetPropertyName();

	const bool bStateChange = PropName == GET_MEMBER_NAME_CHECKED(AStatefulActor, DefaultState);

	if(bStateChange)
	{
		EDITOR_UpdateTagState(DefaultState);
	}
}
#endif

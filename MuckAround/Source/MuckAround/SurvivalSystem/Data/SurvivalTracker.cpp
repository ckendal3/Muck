// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvivalTracker.h"
#include "Engine/World.h"
#include "MuckAround/ActionSystem/Component/ActionComponent.h"
#include "Net/UnrealNetwork.h"

USurvivalTracker::USurvivalTracker()
{
	Identifier		= TAG_SURVIVAL_TRACKER;
	ActionEndType	= EActionEndType::AET_Timer;
	LimitType		= ELimiterType::LT_Unlimited;
	TickRate		= .016f;

	SurvivalTraits.Add(FSurvivalStruct(TAG_SURVIVAL_HUNGER, 100.f));
	SurvivalTraits.Add(FSurvivalStruct(TAG_SURVIVAL_THIRST, 100.f));
	SurvivalTraits.Add(FSurvivalStruct(TAG_SURVIVAL_TEMPERATURE, 100.f));
}

void USurvivalTracker::OnRep_SurvivalTraits()
{
	OnSurvivalTraitChanged();
}

void USurvivalTracker::AdjustSurvivalTrait(FGameplayTag InSurvivalTag, float InValue)
{
	const bool bIsClient = GetWorld()->GetNetMode() == ENetMode::NM_Client;
	if(bIsClient)
	{
		return;	
	}
	
	// Verify its a resource we are adding
	if(InSurvivalTag.MatchesTag(TAG_SURVIVAL))
	{
		// Find a match
		bool bFoundMatch = false;
		for(FSurvivalStruct& SurvivalStruct : SurvivalTraits)
		{
			if(SurvivalStruct.Identifier.MatchesTagExact(InSurvivalTag))
			{
				bFoundMatch = true;
				
				SurvivalStruct.Value = FMath::Max(SurvivalStruct.Value + InValue, 0);
				
				break;
			}
		}

		// Didn't find a match so we need to create it
		if(bFoundMatch == false)
		{
			FSurvivalStruct NewTrait;
			NewTrait.Identifier = InSurvivalTag;
			NewTrait.Value		= FMath::Max(0, InValue);
			
			SurvivalTraits.Add(NewTrait);
		}

		OnSurvivalTraitChanged();
	}
}

void USurvivalTracker::ApplyTraitDelta()
{
	for(const FSurvivalStruct& SurvivalStruct : TraitsDeltas)
	{
		AdjustSurvivalTrait(SurvivalStruct.Identifier, SurvivalStruct.Value * TickRate);
	}
}

void USurvivalTracker::InitializeAction(UActionComponent* Owner)
{
	Super::InitializeAction(Owner);
	
	Owner->StartAction(this);
}

void USurvivalTracker::TickAction_Implementation()
{
	Super::TickAction_Implementation();

	// Apply delta per a tick (typically decays a value)
	ApplyTraitDelta();
}

void USurvivalTracker::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USurvivalTracker, SurvivalTraits);
}

void USurvivalTracker::OnSurvivalTraitChanged_Implementation() { }

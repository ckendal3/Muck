// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceTracker.h"
#include "Engine/World.h"
#include "MuckAround/ActionSystem/Component/ActionComponent.h"
#include "Net/UnrealNetwork.h"

UResourceTracker::UResourceTracker()
{
	Identifier = TAG_RESOURCE_TRACKER;
	ActionEndType = EActionEndType::AET_Manual;
}

void UResourceTracker::OnRep_Resources()
{
	OnResourcesChanged();
}

void UResourceTracker::AddResource(FGameplayTag InResourceTag, float InValue)
{
	const bool bIsClient = GetWorld()->GetNetMode() == ENetMode::NM_Client;
	if(bIsClient)
	{
		return;	
	}
	
	if(InResourceTag.IsValid())
	{
		FResourceStruct ChangingResource;
		
		// Find a match
		bool bFoundMatch = false;
		for(FResourceStruct& ResourceStruct : Resources)
		{
			if(ResourceStruct.Identifier.MatchesTagExact(InResourceTag))
			{
				bFoundMatch = true;
				ResourceStruct.Value = FMath::Max(ResourceStruct.Value + InValue, 0);

				ChangingResource = ResourceStruct;
				
				break;
			}
		}

		// Didn't find a match so we need to create it
		if(bFoundMatch == false)
		{
			FResourceStruct NewResource;
			NewResource.Identifier = InResourceTag;
			NewResource.Value = FMath::Max(0, InValue);
			
			Resources.Add(NewResource);

			ChangingResource = NewResource;
		}

		OnResourceChanged(ChangingResource.Identifier, ChangingResource.Value);
		OnResourcesChanged();
	}
}

void UResourceTracker::OnResourceChanged_Implementation(FGameplayTag ResourceTag, float NewValue) { }

void UResourceTracker::InitializeAction(UActionComponent* Owner)
{
	Super::InitializeAction(Owner);
	
	Owner->StartAction(this);
}

void UResourceTracker::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UResourceTracker, Resources);
}

void UResourceTracker::OnResourcesChanged_Implementation() { }

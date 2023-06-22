// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "MuckAround/ActionSystem/Component/ActionComponent.h"
#include "MuckAround/ActionSystem/Data/ActionDefinition.h"

void AWeapon::SetConfigsOnActions()
{
	// Make sure we are in a valid state (not being destroyed)
	if(IsValid(this))
	{
		for(auto& ConfigSet : ActionConfigurations)
		{
			UActionDefinition* ActionDefinition = ActionComponent->GetAction(ConfigSet.Key);
			if(ActionDefinition)
			{
				ActionDefinition->SetupFromConfig(ConfigSet.Value);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Could not find action %s on %s."), *ConfigSet.Key.ToString(), *GetClass()->GetName());
			}
		}
	}
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	// Only let server initialize
	if(GetWorld()->GetNetMode() == NM_Client)
	{
		SetConfigsOnActions();
	}
}

void AWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Only let server initialize
	if(GetWorld()->GetNetMode() < NM_Client)
	{
		SetConfigsOnActions();
	}
}
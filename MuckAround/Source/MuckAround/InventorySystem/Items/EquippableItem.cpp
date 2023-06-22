// Fill out your copyright notice in the Description page of Project Settings.


#include "EquippableItem.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "MuckAround/ActionSystem/Component/ActionComponent.h"
#include "MuckAround/ActionSystem/Data/ActionDefinition.h"
#include "MuckAround/Characters/Animation/BaseAnimInstance.h"
#include "MuckAround/InventorySystem/InventoryComponent.h"

AEquippableItem::AEquippableItem()
{
	PrimaryActorTick.bCanEverTick			= true;
	PrimaryActorTick.bStartWithTickEnabled	= false;

	ActionComponent = CreateDefaultSubobject<UActionComponent>("ActionComp");

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("ItemMesh");
	RootComponent = Mesh;
}

// TODO: Make more dynamic action binding (probably just install inputs in this class)
void AEquippableItem::TogglePrimaryAction_Implementation()
{
	if(PrimaryActionTag.IsValid())
	{
		UActionDefinition* Action = ActionComponent->GetAction(PrimaryActionTag);

		if(Action && Action->IsActive() == false)
		{
			ActionComponent->StartAction(Action);
		}
		else if(Action && Action->IsActive())
		{
			ActionComponent->StopAction(Action);
		}
	}
}

void AEquippableItem::ToggleSecondaryAction_Implementation()
{
	if(SecondaryActionTag.IsValid())
	{
		UActionDefinition* Action = ActionComponent->GetAction(SecondaryActionTag);

		if(Action && Action->IsActive() == false)
		{
			ActionComponent->StartAction(Action);
		}
		else if(Action && Action->IsActive())
		{
			ActionComponent->StopAction(Action);
		}
	}
}

void AEquippableItem::ToggleTertiaryAction_Implementation()
{
	if(TertiaryActionTag.IsValid())
	{
		UActionDefinition* Action = ActionComponent->GetAction(TertiaryActionTag);

		if(Action && Action->IsActive() == false)
		{
			ActionComponent->StartAction(Action);
		}
		else if(Action && Action->IsActive())
		{
			ActionComponent->StopAction(Action);
		}
	}
}

void AEquippableItem::Equipped_Implementation()
{
	SetupAttachment();
	SetVisibility(true);

	bIsEquipped = true;

	if(GetOwningCharacter() && GetOwningCharacter()->GetMesh())
	{
		USkeletalMeshComponent* CharacterMesh = GetOwningCharacter()->GetMesh();

		if(UBaseAnimInstance* BaseAnimInstance = Cast<UBaseAnimInstance>(CharacterMesh->GetAnimInstance()))
		{
			BaseAnimInstance->SetAnimDefinition(AnimationDefinition);
		}
	}
}

void AEquippableItem::UnEquipped_Implementation()
{
	if(GetWorld()->GetNetMode() < NM_Client)
	{
		ActionComponent->StopAllActions();
	}
	
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// Only hide if it was attached to a character
	if(GetOwningCharacter())
	{
		SetVisibility(false);
	}

	bIsEquipped = false;
}

void AEquippableItem::SetVisibility(bool bShouldBeVisible)
{
	Mesh->SetVisibility(bShouldBeVisible, true);
}

void AEquippableItem::SetupAttachment()
{
	// TODO: Should we just use Owner->GetComponentByClass and get skeletalmesh component?
	if(ACharacter* Character = GetOwningCharacter())
	{
		USkeletalMeshComponent* MeshToAttachTo = Character->GetMesh();
		
		AttachToComponent(MeshToAttachTo, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachmentSocket);
	}
}

void AEquippableItem::OnRep_Owner()
{
	Super::OnRep_Owner();
	
	// Verify the visibility is correct based on if it is equipped or not - only if attached to a character
	if(GetOwningCharacter())
	{
		UInventoryComponent* InventoryComp = Cast<UInventoryComponent>(Owner->GetComponentByClass(UInventoryComponent::StaticClass()));
		const bool bItemIsEquipped = this == InventoryComp->CurrentEquippable;

		SetVisibility(bItemIsEquipped);
	}
}

void AEquippableItem::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);

	OnRep_Owner();
}

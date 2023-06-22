// Fill out your copyright notice in the Description page of Project Settings.


#include "ADAim.h"
#include "InventoryTags.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MuckAround/Characters/BaseCharacter.h"

UADAim::UADAim()
{
	Identifier		= TAG_WEAPON_AIM;
	ActionEndType	= EActionEndType::AET_Manual;
}

void UADAim::BeginAction_Implementation()
{
	Super::BeginAction_Implementation();

	// TODO: GetCharacter function
	if(ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwningActor()->GetOwner()))
	{
		Character->GetFollowCamera()->bUsePawnControlRotation			= true;
		Character->bUseControllerRotationYaw							= true;
		Character->GetCharacterMovement()->bOrientRotationToMovement	= false;

		StoredMaxWalkSpeed								= Character->GetCharacterMovement()->MaxWalkSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeed	= 150.0f;

		if(IsLocallyOwned())
		{
			APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
			if(IsValid(PlayerController))
			{
				PlayerController->FOV(55.f);
			}
		}
	}
}

void UADAim::EndAction_Implementation()
{
	Super::EndAction_Implementation();

	// TODO: GetCharacter function
	if(ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwningActor()->GetOwner()))
	{
		Character->GetFollowCamera()->bUsePawnControlRotation			= false;
		Character->bUseControllerRotationYaw							= false;
		Character->GetCharacterMovement()->bOrientRotationToMovement	= true;
		Character->GetCharacterMovement()->MaxWalkSpeed					= StoredMaxWalkSpeed;

		if(IsLocallyOwned())
		{
			APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
			if(IsValid(PlayerController))
			{
				PlayerController->FOV(90.f);
			}
		}
	}
}

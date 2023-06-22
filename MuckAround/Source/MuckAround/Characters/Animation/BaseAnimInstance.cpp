// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAnimInstance.h"

#include "AnimationDefinition.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBaseAnimInstance::UBaseAnimInstance()
{
	MovementMode = EMovementMode::MOVE_Walking;
}

ACharacter* UBaseAnimInstance::GetOwningCharacter() const
{
	return Cast<ACharacter>(GetOwningActor());
}

void UBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if(GetWorld() && GetWorld()->IsGameWorld())
	{
		ensure(GetOwningCharacter());
	}

	if(GetOwningCharacter())
	{
		Velocity      = GetOwningCharacter()->GetVelocity();
		LookDirection = GetOwningCharacter()->GetBaseAimRotation();
		MovementMode  = GetOwningCharacter()->GetCharacterMovement()->MovementMode;
	}

}

float UBaseAnimInstance::GetAimPitch() const
{
	// Adjust our pitch value to be a proper format
	if(const ACharacter* Char = GetOwningCharacter())
	{
		const float OriginalPitch = Char->GetBaseAimRotation().Pitch;

		return OriginalPitch > 90.f ? OriginalPitch - 360.f : OriginalPitch;
	}

	return 0.f;
}

float UBaseAnimInstance::GetRelativeMovementDirection() const
{
	// Used for blendspaces
	if(const ACharacter* Char = GetOwningCharacter())
	{
		return CalculateDirection(GetVelocity(), Char->GetActorForwardVector().Rotation());
	}

	return 0.f;
}

void UBaseAnimInstance::SetAnimDefinition(UAnimationDefinition* NewDefinition)
{
	UAnimationDefinition* PreviousDefinition = GetAnimDefinition();

	AnimDefinition = NewDefinition;

	OnNewAnimDefinitionSet(PreviousDefinition, NewDefinition);
}

void UBaseAnimInstance::OnNewAnimDefinitionSet_Implementation(UAnimationDefinition* OldDefinition,
															  UAnimationDefinition* NewDefinition) { }

UAnimMontage* UBaseAnimInstance::PlayAnimationByTag(FGameplayTag InTag, FName SlotName, float PlayRate)
{
	// TODO: Implement random animation/index 
	if(IsValid(GetAnimDefinition()))
	{
		UAnimSequenceBase* AnimSequence = GetAnimDefinition()->GetAnimation(InTag);

		return PlayAnimation(AnimSequence, SlotName, PlayRate);
	}

	return nullptr;
}

UAnimMontage* UBaseAnimInstance::PlayAnimation(UAnimSequenceBase* Animation, FName SlotName, float PlayRate)
{
	// TODO: Have a better config setup so playrate, start location, looping, etc can be specified
	if(IsValid(Animation))
	{
		if(Animation->IsA(UAnimMontage::StaticClass()))
		{
			UAnimMontage* AnimMontage = CastChecked<UAnimMontage>(Animation);
			Montage_Play(AnimMontage, PlayRate);

			return AnimMontage;
		}
		else if(Animation->IsA(UAnimSequence::StaticClass()))
		{
			return PlaySlotAnimationAsDynamicMontage(Animation, SlotName, .1f, .1f, PlayRate);
		}
	}

	return nullptr;
}



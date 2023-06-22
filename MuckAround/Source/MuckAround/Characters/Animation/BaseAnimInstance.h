// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimInstance.h"
#include "BaseAnimInstance.generated.h"

class UAnimSequenceBase;
class UAnimationDefinition;

// TODO: Add a gameplay tag layer so we can just pass in Tags like (IsCrouching, IsJumping, etc)
//       and store them so we can query as needed

/**
 * Base animation layer for characters.
 */
UCLASS()
class UBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	UBaseAnimInstance();

public:

	UFUNCTION(BlueprintPure)
	class ACharacter* GetOwningCharacter() const;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	FRotator GetLookDirection() const { return LookDirection; }

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	FVector GetLookDirectionAsVector() const { return LookDirection.Vector(); }

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	FVector GetVelocity() const { return Velocity; }
	
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	float GetSpeed() const { return Velocity.Size(); }
	
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	float GetGroundSpeed() const { return Velocity.Size2D(); }
	
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	float GetSpeedFalling() const { return Velocity.Z; }

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	bool IsFalling() const { return MovementMode == EMovementMode::MOVE_Falling; }

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	bool IsInMovementMode(EMovementMode DesiredMode) const { return MovementMode == DesiredMode; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Meta = (BlueprintThreadSafe))
	float GetAimPitch() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Meta = (BlueprintThreadSafe))
	float GetRelativeMovementDirection() const;

	UFUNCTION(BlueprintPure)
	UAnimationDefinition* GetAnimDefinition() const { return AnimDefinition; }
	
	UFUNCTION(BlueprintCallable)
	void SetAnimDefinition(UAnimationDefinition* NewDefinition);
	
	UFUNCTION(BlueprintNativeEvent)
	void OnNewAnimDefinitionSet(UAnimationDefinition* OldDefinition, UAnimationDefinition* NewDefinition);
	
	UAnimMontage* PlayAnimationByTag(FGameplayTag InTag, FName SlotName = FName(""), float PlayRate = 1.f);

	UAnimMontage* PlayAnimation(UAnimSequenceBase* Animation, FName SlotName = FName(""), float PlayRate = 1.f);
	
protected:

	UPROPERTY(EditAnywhere)
	UAnimationDefinition* AnimDefinition;
	
	UPROPERTY(EditDefaultsOnly, Category = "Core")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "Core")
	FRotator LookDirection = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, Category = "Core")
	TEnumAsByte<EMovementMode> MovementMode;
};
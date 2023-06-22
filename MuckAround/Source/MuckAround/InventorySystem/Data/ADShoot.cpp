// Fill out your copyright notice in the Description page of Project Settings.


#include "ADShoot.h"
#include "InventoryTags.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MuckAround/ActionSystem/Component/ActionComponent.h"
#include "MuckAround/Characters/BaseCharacter.h"
#include "MuckAround/Characters/Animation/BaseAnimInstance.h"
#include "MuckAround/InventorySystem/Config/InventoryConfig.h"
#include "MuckAround/InventorySystem/Weapons/Weapon.h"

UADShoot::UADShoot()
{
	Identifier		= TAG_WEAPON_SHOOT;
	ActionEndType	= EActionEndType::AET_Manual;

	OperatingTags.AddTag(TAG_WEAPON_SHOOT);
	SortingTags.AddTag(TAG_WEAPON_SHOOT);

	BlockingTags.AddTag(TAG_WEAPON_MELEE);

	MontageEndedDelegate.BindUObject(this, &UADShoot::OnMontageEnded);

	DamageType = UDamageType::StaticClass();
}

void UADShoot::SetupFromConfig(UActionConfig* InConfig)
{
	if(IsValid(InConfig))
	{
		const UShootConfig* ShootConfig = Cast<UShootConfig>(InConfig);

		ensureMsgf(IsValid(ShootConfig), TEXT("Received something that wasn't UShootConfig!"));
		if(IsValid(ShootConfig))
		{
			DamageAmount	= ShootConfig->DamageAmount;
			DamageType		= ShootConfig->DamageType;
			Count			= ShootConfig->Count;
			Spread			= ShootConfig->Spread;
			Range			= ShootConfig->Range;
			bMultiHit		= ShootConfig->bMultiHit;
			SocketName		= ShootConfig->SocketName;
		}
	}
}

void UADShoot::BeginAction_Implementation()
{	
	FVector StartLocation;
	FVector StaticDirection;

	GetShootLocationAndDirection(StartLocation, StaticDirection);

	FVector DesiredAimLocation;
	GetDesiredAimLocation(DesiredAimLocation);

	StaticDirection = GetAimDirectionToLocation(StartLocation, DesiredAimLocation).Vector();
	
	float DegreesAsRadians = FMath::DegreesToRadians(Spread);


	TArray<FHitResult> HitResults;
	
	for(int i = 0; i < Count; i++)
	{
		const FVector RandDirection = FMath::VRandCone(StaticDirection, DegreesAsRadians,
										DegreesAsRadians);
		
		const FVector EndLocation = StartLocation + (RandDirection * Range);

		FCollisionQueryParams QueryParams("Shoot");
		QueryParams.AddIgnoredActor(GetOwningActor());
		QueryParams.AddIgnoredActor(GetOwningActor()->GetOwner());
		QueryParams.bTraceComplex = false;
		
		if(bMultiHit)
		{
			GetWorld()->LineTraceMultiByProfile(HitResults, StartLocation, EndLocation,
													UCollisionProfile::DefaultProjectile_ProfileName, QueryParams);
		}
		else
		{
			FHitResult SingleHitResult;
			GetWorld()->LineTraceSingleByProfile(SingleHitResult, StartLocation, EndLocation,
													UCollisionProfile::DefaultProjectile_ProfileName, QueryParams);

			HitResults.Add(SingleHitResult);
		}
	}

	for(FHitResult& HitResult : HitResults)
	{
		OnHitReceived(HitResult);
	}

	SpawnMuzzleFlash();
	SpawnVFXForShots(HitResults);

	if(ACharacter* Character = Cast<ACharacter>(GetOwningActor()->GetOwner()))
	{
		if(USkeletalMeshComponent* CharacterMesh = Character->GetMesh())
    	{
    		if(UBaseAnimInstance* BaseAnimInstance = Cast<UBaseAnimInstance>(CharacterMesh->GetAnimInstance()))
    		{
    			AnimMontage = BaseAnimInstance->PlayAnimationByTag(Identifier);

    			if(AnimMontage)
    			{
    				BaseAnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, AnimMontage);
    			}
    		}
    	}
	}
}

bool UADShoot::CanStart_Implementation() const
{
	return Super::CanStart_Implementation()
			&& IsValid(AnimMontage) == false
			&& OwningComponent->IsActionRunning(TAG_WEAPON_AIM);
}

FRotator UADShoot::GetAimDirectionToLocation(const FVector& FromLocation, const FVector& LookAtLocation) const
{
	return UKismetMathLibrary::FindLookAtRotation(FromLocation, LookAtLocation);
}

void UADShoot::SpawnMuzzleFlash()
{
	AWeapon* Weapon = Cast<AWeapon>(GetOwningActor());
	if(IsValid(Weapon) && IsValid(Weapon->MuzzleFlashEffect))
	{
		UNiagaraSystem* VFXSystem = Weapon->MuzzleFlashEffect;

		FVector		ShootLoc;
		FVector		ShootRot;
		GetShootLocationAndDirection(ShootLoc, ShootRot);
		
		UNiagaraComponent* MuzzleFlash = UNiagaraFunctionLibrary::SpawnSystemAttached(VFXSystem, Weapon->Mesh, FName("Muzzle"),
																					FVector::ZeroVector, Weapon->GetActorForwardVector().Rotation(),
																					EAttachLocation::SnapToTarget, true);
		
		MuzzleFlash->SetNiagaraVariableBool(FString("User.Trigger"), true);
	}
}

void UADShoot::SpawnVFXForShots(TArray<FHitResult>& InArray)
{
	AWeapon* Weapon = Cast<AWeapon>(GetOwningActor());
	if(IsValid(Weapon) && IsValid(Weapon->TracerEffect))
	{
		UNiagaraSystem* VFXSystem = Weapon->TracerEffect;
		
		for(FHitResult& HitResult : InArray)
		{
			UNiagaraComponent* Tracer = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, VFXSystem, HitResult.TraceStart);
			
			Tracer->SetNiagaraVariableBool(FString("User.Trigger"), true);

			const FVector EndPos = HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd;

			TArray<FVector> ImpactPositions;
			ImpactPositions.Add(EndPos);
			
			UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
				Tracer,
				FName("User.ImpactPositions"),
				ImpactPositions
			);
		}
	}
}

void UADShoot::GetDesiredAimLocation_Implementation(FVector& InLocation) const
{
	AWeapon* Weapon = Cast<AWeapon>(GetOwningActor());
	
	if(ensure(IsValid(Weapon)) == false)
	{
		return;
	}
	
	FVector		LookLoc;
	FRotator	LookRot;

	// Try to use an accurate look data
	if(IsValid(Weapon->GetOwningCharacter()))
	{
		ACharacter* Character = Weapon->GetOwningCharacter();
		
		if(IsValid(Character->GetController()))
		{
			GetController()->GetPlayerViewPoint(LookLoc, LookRot);
		}
		else
		{
			Character->GetActorEyesViewPoint(LookLoc, LookRot);
			LookRot = Character->GetBaseAimRotation();
		}
	}
	else
	{
		// Fallback as the weapon should be able to shoot no matter what
		LookLoc = GetOwningActor()->GetActorLocation();
		LookRot = GetOwningActor()->GetActorRotation();
	}
	
	FVector EndLocation = LookLoc + (LookRot.Vector() * 8192);

	FCollisionQueryParams QueryParams("Aim");
	QueryParams.AddIgnoredActor(GetOwningActor());
	QueryParams.AddIgnoredActor(GetOwningActor()->GetOwner());
	QueryParams.bTraceComplex = false;
	
	FHitResult SingleHitResult;
	bool bHasHit = GetWorld()->LineTraceSingleByProfile(SingleHitResult, LookLoc, EndLocation,
											UCollisionProfile::DefaultProjectile_ProfileName, QueryParams);
	
	InLocation = bHasHit ? SingleHitResult.ImpactPoint : EndLocation;
}

void UADShoot::GetShootLocationAndDirection_Implementation(FVector& OutLocation, FVector& OutDirection) const
{
	const AActor* Owner = GetOwningActor();

	OutLocation		= Owner->GetActorLocation();
	OutDirection	= Owner->GetActorForwardVector();

	if(USkeletalMeshComponent* MeshComponent = Cast<USkeletalMeshComponent>(Owner->GetComponentByClass(USkeletalMeshComponent::StaticClass())))
	{
		if(MeshComponent->DoesSocketExist(SocketName))
		{
			FRotator Rot; 
			MeshComponent->GetSocketWorldLocationAndRotation(SocketName, OutLocation, Rot);

			OutDirection = Rot.Vector();
		}
	}
}

void UADShoot::OnHitReceived_Implementation(FHitResult& HitResult)
{
	if(IsValid(HitResult.GetActor()))
	{
		AActor* HitActor = HitResult.GetActor();
	
		UGameplayStatics::ApplyDamage(HitActor, DamageAmount, GetController(), GetOwningActor(), DamageType);
	}
}

AController* UADShoot::GetController_Implementation() const
{
	return GetOwningActor()->GetInstigatorController();
}

void UADShoot::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if(Montage == AnimMontage)
	{
		AnimMontage = nullptr;
		
		Request_InternalEnd();
	}
}
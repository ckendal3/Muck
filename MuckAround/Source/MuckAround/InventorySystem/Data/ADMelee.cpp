// Fill out your copyright notice in the Description page of Project Settings.


#include "ADMelee.h"
#include "InventoryTags.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "MuckAround/ActionSystem/Component/ActionComponent.h"
#include "MuckAround/ActionSystem/Data/ActionConfig.h"
#include "MuckAround/Characters/Animation/BaseAnimInstance.h"

UADMelee::UADMelee() : Super()
{
	Identifier = TAG_WEAPON_MELEE;

	ActionEndType	= EActionEndType::AET_Timer;
	LimitType		= ELimiterType::LT_Unlimited;

	TickRate		= .016f;

	OperatingTags.AddTag(TAG_WEAPON_MELEE);
	SortingTags.AddTag(TAG_WEAPON_MELEE);

	BlockingTags.AddTag(TAG_WEAPON_SHOOT);

	MontageEndedDelegate.BindUObject(this, &UADMelee::OnMontageEnded);

	DamageType = UDamageType::StaticClass();
}

void UADMelee::SetupFromConfig(UActionConfig* InConfig)
{
	if(IsValid(InConfig))
	{
		UDamageConfig* ShootConfig = Cast<UDamageConfig>(InConfig);

		ensureMsgf(IsValid(ShootConfig), TEXT("Received something that wasn't UDamageConfig!"));
		if(IsValid(ShootConfig))
		{
			DamageAmount	= ShootConfig->DamageAmount;
			DamageType		= ShootConfig->DamageType;
		}
	}
}

void UADMelee::InitializeAction(UActionComponent* Owner)
{
	Super::InitializeAction(Owner);

	// Initialize the same as the client does
	OnRep_OwningComponent();
}

void UADMelee::BeginAction_Implementation()
{
	Hits.Empty();
	AlreadyHitActors.Empty();

	InitializeSocketData();
	
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
					return;
				}
			}
		}
	}

#if UE_BUILD_DEVELOPMENT
	// We should never reach here
	ensureMsgf(AnimMontage, TEXT("AnimMontage was nullptr, we should never have this state."));
	Request_InternalEnd();
#endif
}

bool UADMelee::CanStart_Implementation() const
{	
	return Super::CanStart_Implementation()
			&& IsValid(AnimMontage) == false
			&& OwningComponent->IsActionRunning(TAG_WEAPON_AIM);
}

void UADMelee::TickAction_Implementation()
{
	UpdateSocketData();
	PerformTraceCheck();
	ProcessHits();
}

void UADMelee::OnRep_OwningComponent()
{
	TrackedMesh = Cast<USkeletalMeshComponent>(GetOwningActor()->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
	
	if(TrackedMesh)
	{
		for(auto SocketName : Sockets)
		{
			if(TrackedMesh->DoesSocketExist(SocketName))
			{
				const FVector SocketLocation = TrackedMesh->GetSocketLocation(SocketName);
				
				SocketData.Add(FSocketData(SocketName, SocketLocation));
			}
		}
	}
}

void UADMelee::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if(Montage == AnimMontage)
	{
		AnimMontage = nullptr;

		Request_InternalEnd();
	}
}

void UADMelee::InitializeSocketData()
{
	if(TrackedMesh)
	{
		for(FSocketData& SocketDatum : SocketData)
		{
			// Update our info to be up to date on action begin
			SocketDatum.CurrentLocation		= TrackedMesh->GetSocketLocation(SocketDatum.SocketName);
			SocketDatum.PreviousLocation	= SocketDatum.CurrentLocation;
		}
	}
}

void UADMelee::UpdateSocketData()
{
	if(TrackedMesh)
	{
		for(FSocketData& SocketDatum : SocketData)
		{
			SocketDatum.PreviousLocation	= SocketDatum.CurrentLocation;
			SocketDatum.CurrentLocation		= TrackedMesh->GetSocketLocation(SocketDatum.SocketName);
		}
	}
}

void UADMelee::PerformTraceCheck()
{
	// TODO: Implement proper profile
	// TODO: Implement different shapes, size, orientation
	for(const FSocketData& SocketDatum : SocketData)
	{
		FCollisionShape Shape = FCollisionShape::MakeSphere(10.f);

		FCollisionQueryParams QueryParams("Melee");
		QueryParams.AddIgnoredActor(GetOwningActor());
		QueryParams.AddIgnoredActor(GetOwningActor()->GetOwner());

		TArray<FHitResult> HitResults;

		GetWorld()->SweepMultiByProfile(HitResults, SocketDatum.PreviousLocation, SocketDatum.CurrentLocation,
										FQuat::Identity, UCollisionProfile::DefaultProjectile_ProfileName, Shape, QueryParams);

		Hits.Append(HitResults);
	}
}

void UADMelee::ProcessHits()
{
	for(FHitResult& HitResult : Hits)
	{
		AActor* HitActor = HitResult.GetActor();
		
		if(IsValid(HitActor) && AlreadyHitActors.Contains(HitActor) == false)
		{
			AlreadyHitActors.Add(HitActor);

			UGameplayStatics::ApplyDamage(HitActor, DamageAmount, GetController(), GetOwningActor(), DamageType);
			
			OnProcessHit(HitActor);
		}
	}
}

AController* UADMelee::GetController_Implementation() const
{
	return GetOwningActor()->GetInstigatorController();
}
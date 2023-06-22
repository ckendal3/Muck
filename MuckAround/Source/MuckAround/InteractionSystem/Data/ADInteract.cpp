// Fill out your copyright notice in the Description page of Project Settings.


#include "ADInteract.h"
#include "GameFramework/Character.h"
#include "MuckAround/ActionSystem/Component/ActionComponent.h"
#include "MuckAround/InteractionSystem/InteractionInterface.h"
#include "MuckAround/InteractionSystem/InteractionTags.h"
#include "MuckAround/InteractionSystem/Config/InteractionConfig.h"

UADInteract::UADInteract()
{
	Identifier = TAG_INTERACTION;
	
	ActionEndType	= EActionEndType::AET_Timer;
	LimitType		= ELimiterType::LT_Unlimited;

	TickRate		= .016f;

	OperatingTags.AddTag(TAG_INTERACTION);
	SortingTags.AddTag(TAG_INTERACTION);

	HeldDelegate.BindUObject(this, &UADInteract::InteractionHeld);
}

void UADInteract::GetOwnerViewAndLocation_Implementation(FVector& OutLocation, FRotator& OutDirection)
{
	if(ACharacter* Character = Cast<ACharacter>(GetOwningActor()))
	{
		Character->GetActorEyesViewPoint(OutLocation, OutDirection);
	}
}

void UADInteract::SetupFromConfig(UActionConfig* InConfig)
{
	if(IsValid(InConfig))
	{
		const UInteractionConfig* InteractionConfig = Cast<UInteractionConfig>(InConfig);

		ensureMsgf(IsValid(InteractionConfig), TEXT("Received something that wasn't UInteractionConfig!"));
		if(IsValid(InteractionConfig))
		{
			OverlapCheckSize	= InteractionConfig->OverlapCheckSize;
		}
	}
}

void UADInteract::InitializeAction(UActionComponent* Owner)
{
	Super::InitializeAction(Owner);

	Owner->StartAction(this);
}

void UADInteract::TickAction_Implementation()
{
	const FVector	Location = GetOwningActor()->GetActorLocation();
	const FQuat		Rotation = GetOwningActor()->GetActorRotation().Quaternion();

	const bool bIsBeingHeld = CurrentStateTag == TAG_INTERACTION_HELD;

	if(IsValid(ValidActor) == false || bIsBeingHeld == false)
	{
		// Set up our overlap data
    	FCollisionQueryParams QueryParams("Interact");
    	QueryParams.AddIgnoredActor(GetOwningActor());
    	QueryParams.AddIgnoredActor(GetOwningActor()->GetOwner());
    	QueryParams.bTraceComplex = false;
    	
    	const FCollisionShape CollisionShape = FCollisionShape::MakeSphere(OverlapCheckSize);
    
    	TArray<FOverlapResult> OverlapResults;
    	
    	GetWorld()->OverlapMultiByProfile(OverlapResults, Location, Rotation, UCollisionProfile::DefaultProjectile_ProfileName,
    										CollisionShape, QueryParams);
    
    	FVector		LookFromLocation;
    	FRotator	Direction;
    
    	GetOwnerViewAndLocation(LookFromLocation, Direction);
    	
    	float ClosestDot = DotFromOwnerViewAndLocation(ValidActor, LookFromLocation, Direction);
    	if(OverlapResults.Num() > 0)
    	{
    		// Find the closest target to our look direction
    		for(auto& Result : OverlapResults)
    		{
    			// Makes sure its a valid actor implementing desired interface
    			AActor* OverlappedActor = Result.GetActor(); 
				if(IsValid(OverlappedActor) == false || OverlappedActor->Implements<UInteractionInterface>() == false)
				{
					continue;
				}
    			
    			float DotValue = DotFromOwnerViewAndLocation(OverlappedActor, LookFromLocation, Direction);
    			
    			if(IsValid(ValidActor) == false || DotValue > ClosestDot)
    			{
    				ClosestDot = DotValue;
    				SetTargetActor(OverlappedActor);
    
    				// Its not going to get better than this, early out
    				if(ClosestDot == 1.f)
    				{
    					break;
    				}
    			}
    		}
    	}
    	else
    	{
    		ClearTargetActor();
    	}	
	}
}

void UADInteract::CustomStateAction_Implementation()
{	
	// Stop ticking and do holding
	if(CurrentStateTag == TAG_INTERACTION_HELD)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

		GetWorld()->GetTimerManager().SetTimer(HeldHandle, HeldDelegate, TickRate, true, 0.f);
	}
	else // restart our ticking
	{
		GetWorld()->GetTimerManager().ClearTimer(HeldHandle);

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, TickRate, true, 0.f);
	}
	
	AttemptInteraction();
}

float UADInteract::DotFromOwnerViewAndLocation(AActor* Actor, const FVector& LookFromLocation, const FRotator& Direction)
{
	float DotValue = -1.f;
	
	if(IsValid(Actor))
	{
		const FVector DirToResult = (Actor->GetActorLocation() - LookFromLocation).GetSafeNormal();
		DotValue = FVector::DotProduct(DirToResult, Direction.Vector());
	}

	return DotValue;
}

void UADInteract::SetTargetActor(AActor* TargetActor)
{
	if(IsValid(TargetActor) && TargetActor->Implements<UInteractionInterface>())
	{
		ValidActor = TargetActor;
	}
}

void UADInteract::ClearTargetActor()
{
	ValidActor	= nullptr;
	HoldTime	= 0.f;
}

void UADInteract::AttemptInteraction()
{
	// Make sure the previous interactable is still within range
	if(IsValid(ValidActor) && FVector::Distance(GetOwningActor()->GetActorLocation(), ValidActor->GetActorLocation()) > OverlapCheckSize)
	{
		ClearTargetActor();
	}
	
	if(IsValid(ValidActor))
	{
		FGameplayTag	DesiredState;
		float			DesiredHoldTime;

		IInteractionInterface::Execute_GetDesiredInteraction(ValidActor, DesiredState, DesiredHoldTime);

		bool bIsInteractTag =	CurrentStateTag == TAG_INTERACTION_HELD
							||	CurrentStateTag == TAG_INTERACTION_RELEASED;
		
		if(bIsInteractTag)
		{
			IInteractionInterface::Execute_SetInteractionState(ValidActor, CurrentStateTag, HoldTime, this);
			
			if(HoldTime >= DesiredHoldTime || CurrentStateTag == TAG_INTERACTION_RELEASED)
			{
				ClearTargetActor();
			}
		}
	}
}

void UADInteract::InteractionHeld()
{
	if(IsValid(ValidActor))
	{		
		HoldTime += TickRate;
		AttemptInteraction();
	}
}

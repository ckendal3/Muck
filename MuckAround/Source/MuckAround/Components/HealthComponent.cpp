// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthComponent.h"
#include "Engine/TextRenderActor.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bCanEverAffectNavigation = false;
	bWantsInitializeComponent = true;
	
	SetIsReplicatedByDefault(true);
}

void UHealthComponent::OnDamageReceived(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	FDamageEvent DamageEvent;
	DamageEvent.DamageTypeClass = DamageType->GetClass();
	
	if(ShouldTakeDamage(Damage, DamageEvent, InstigatedBy, DamageCauser))
	{
		TakeDamage(DamagedActor, Damage, DamageType, InstigatedBy, DamageCauser);
	}
}

bool UHealthComponent::ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	const bool bCanBeDamaged = Damage > 0 && CurrentHealth > 0;
	const bool bCanBeHealed  = Damage < 0 && CurrentHealth < MaxHealth;
	
	return IsAlive() && (bCanBeDamaged || bCanBeHealed);
}

void UHealthComponent::TakeDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser )
{
	if(IsAlive())
	{
		const float PreDamageHealth = CurrentHealth;
		
		CurrentHealth = CurrentHealth - Damage;

		if(GetWorld()->GetNetMode() < ENetMode::NM_Client)
		{
			OnRep_CurrentHealth(PreDamageHealth);
		}

		if(OnHealthChanged.IsBound())
		{
			OnHealthChanged.Broadcast(PreDamageHealth, CurrentHealth);
		}

		if(IsDead() && OnDeath.IsBound())
		{
			OnDeath.Broadcast(GetOwner());
		}
	}
}

void UHealthComponent::OnRep_CurrentHealth(float OldValue)
{
	UE_LOG(LogTemp, Warning, TEXT("OldValue %f, NewValue %f"), OldValue, CurrentHealth);
	HealthValueChanged(OldValue - CurrentHealth);
}

void UHealthComponent::HealthValueChanged_Implementation(float Difference) { }

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, CurrentHealth);
}

void UHealthComponent::OnRegister()
{
	Super::OnRegister();

	GetOwner()->OnTakeAnyDamage.AddUniqueDynamic(this, &UHealthComponent::OnDamageReceived);
}
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DamageEvents.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathEvent, AActor*, NewlyDeadActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangeEvent, float, OldValue, float, NewValue);

UENUM(BlueprintType)
enum class ETextType : uint8
{
	ETT_Stack,
	ETT_Individual
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxHealth = 100.f;

	UPROPERTY(EditDefaultsOnly)
	ETextType TextType = ETextType::ETT_Individual;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth = 100.f;

	UPROPERTY(BlueprintAssignable)
	FOnHealthChangeEvent OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnDeathEvent OnDeath;
	
	UHealthComponent();

	bool IsAlive() const { return CurrentHealth > 0.f;  }
	bool IsDead()  const { return CurrentHealth <= 0.f; }

	UFUNCTION()
	void OnDamageReceived(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	
	virtual bool ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const;
	void TakeDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	virtual void OnRep_CurrentHealth(float OldValue);

	UFUNCTION(BlueprintNativeEvent)
	void HealthValueChanged(float Difference);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRegister() override;
};
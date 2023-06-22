// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MuckAround/InventorySystem/Items/EquippableItem.h"
#include "Weapon.generated.h"

class UActionConfig;

UCLASS()
class MUCKAROUND_API AWeapon : public AEquippableItem
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, UActionConfig*> ActionConfigurations;

	void SetConfigsOnActions();
	
	virtual void OnRep_Owner() override;
	virtual void PostInitializeComponents() override;

public:

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* MuzzleFlashEffect;
	
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TracerEffect;
};

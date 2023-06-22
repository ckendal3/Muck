// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/HitResult.h"
#include "MuckAround/ActionSystem/Data/ActionDefinition.h"
#include "ADShoot.generated.h"

class UDamageType;
class AController;
class UActionConfig;

// TODO: Pass config from weapon to initialize the action

/**
 * 
 */
UCLASS()
class MUCKAROUND_API UADShoot : public UActionDefinition
{
	GENERATED_BODY()

public:

	UADShoot();
	
protected:

	UPROPERTY(EditDefaultsOnly)
		float DamageAmount = 10.f;
	
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly)
		int Count = 1;
		
	UPROPERTY(EditDefaultsOnly)
		float Spread = 10.f;

	UPROPERTY(EditAnywhere)
		float Range = 8192.f;
	
	UPROPERTY(EditDefaultsOnly)
		bool bMultiHit = false;
		
	UPROPERTY(EditDefaultsOnly)
		FName SocketName = FName("Muzzle");
	
	virtual void SetupFromConfig(UActionConfig* InConfig) override;
	virtual void BeginAction_Implementation() override;
	virtual bool CanStart_Implementation() const override;

	UFUNCTION(BlueprintNativeEvent)
		void OnHitReceived(FHitResult& HitResult);

	UFUNCTION(BlueprintNativeEvent)
		void GetShootLocationAndDirection(FVector& Location, FVector& Direction) const;

	UFUNCTION(BlueprintNativeEvent)
		AController* GetController() const;

	UFUNCTION(BlueprintNativeEvent)
		void GetDesiredAimLocation(FVector& InLocation) const;
	
	FRotator GetAimDirectionToLocation(const FVector& FromLocation, const FVector& LookAtLocation) const;

	void SpawnMuzzleFlash();
	void SpawnVFXForShots(TArray<FHitResult>& InArray);
	
	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<UAnimMontage> AnimMontage;

	FOnMontageEnded MontageEndedDelegate;
	
	UFUNCTION()
		void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};

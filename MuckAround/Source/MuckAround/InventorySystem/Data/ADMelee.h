// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MuckAround/ActionSystem/Data/ActionDefinition.h"
#include "ADMelee.generated.h"

class UActionConfig;
class USkeletalMeshComponent;

USTRUCT()
struct FSocketData
{
	GENERATED_BODY()

	FSocketData()
	{
		SocketName			= FName("");
		PreviousLocation	= FVector::ZeroVector;
		CurrentLocation		= FVector::ZeroVector;
	}

	FSocketData(FName InSocketName, FVector Location)
	{
		SocketName			= InSocketName;
		PreviousLocation	= Location;
		CurrentLocation		= Location;
	}
	
	UPROPERTY(VisibleInstanceOnly)
	FName SocketName;

	UPROPERTY(VisibleInstanceOnly)
	FVector PreviousLocation;

	UPROPERTY(VisibleInstanceOnly)
	FVector CurrentLocation;
};

/**
 * 
 */
UCLASS()
class MUCKAROUND_API UADMelee : public UActionDefinition
{
	GENERATED_BODY()

public:

	UADMelee();

	UPROPERTY(EditDefaultsOnly)
	float DamageAmount = 10.f;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere)
	TArray<FName> Sockets;

	UPROPERTY(VisibleInstanceOnly)
	TArray<FSocketData> SocketData;
	
	UPROPERTY(VisibleInstanceOnly)
	TArray<FHitResult> Hits;

	UPROPERTY(VisibleInstanceOnly)
	TArray<AActor*> AlreadyHitActors;

	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<UAnimMontage> AnimMontage;

	FOnMontageEnded MontageEndedDelegate;

	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<USkeletalMeshComponent> TrackedMesh;
	
	virtual	void SetupFromConfig(UActionConfig* InConfig) override;
	
	virtual void InitializeAction(UActionComponent* Owner) override;
	virtual void BeginAction_Implementation() override;
	virtual bool CanStart_Implementation() const override;
	virtual void TickAction_Implementation() override;

	virtual void OnRep_OwningComponent() override;

	UFUNCTION()
		void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void InitializeSocketData();
	void UpdateSocketData();
	void PerformTraceCheck();
	void ProcessHits();

	UFUNCTION(BlueprintImplementableEvent)
	void OnProcessHit(AActor* HitActor);

	UFUNCTION(BlueprintNativeEvent)
		AController* GetController() const;
};

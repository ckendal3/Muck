// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MuckAround/ActionSystem/Data/ActionConfig.h"
#include "InventoryConfig.generated.h"

/**
 * 
 */
UCLASS()
class MUCKAROUND_API UShootConfig : public UDamageConfig
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	int Count = 1;
		
	UPROPERTY(EditAnywhere)
	float Spread = 10.f;

	UPROPERTY(EditAnywhere)
	float Range = 8192.f;

	UPROPERTY(EditAnywhere)
	bool bMultiHit = false;
		
	UPROPERTY(EditAnywhere)
	FName SocketName = FName("Muzzle");	
};
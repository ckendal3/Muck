// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "InventoryItem.generated.h"

class ACharacter;

UCLASS()
class MUCKAROUND_API AInventoryItem : public AActor
{
	GENERATED_BODY()

public:

	AInventoryItem();

	FORCEINLINE ACharacter* GetOwningCharacter() const;
};

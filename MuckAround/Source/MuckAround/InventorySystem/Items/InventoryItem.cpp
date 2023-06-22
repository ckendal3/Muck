// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItem.h"
#include "GameFramework/Character.h"


AInventoryItem::AInventoryItem()
{
	PrimaryActorTick.bCanEverTick			= false;
	PrimaryActorTick.bStartWithTickEnabled	= false;
	
	bReplicates = true;
}

ACharacter* AInventoryItem::GetOwningCharacter() const
{
	return Cast<ACharacter>(Owner);
}

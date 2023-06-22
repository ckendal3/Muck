// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Items/EquippableItem.h"
#include "Items/InventoryItem.h"
#include "Net/UnrealNetwork.h"


UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::InitializeDefaultInventory()
{
	const bool bIsClient = GetWorld()->GetNetMode() == NM_Client;

	ensureMsgf(bIsClient == false, TEXT("Client should never initialize default inventory"));
	if(bIsClient)
	{
		return;
	}

	// Create all items
	if(DefaultItemsArray.Num() > 0)
	{
		for(const auto ItemClass : DefaultItemsArray)
		{
			AInventoryItem* NewItem = CreateItem(ItemClass);
			if(NewItem)
			{
				InventoryItems.Add(NewItem);
			}
		}
	}

	// Equip first valid item
	for(const auto ItemPtr : InventoryItems)
	{
		AEquippableItem* EquippableItem = Cast<AEquippableItem>(ItemPtr);
		if(EquippableItem)
		{
			EquipItem(EquippableItem);
			break;
		}
	}
}

void UInventoryComponent::EquipItem(AEquippableItem* ItemToEquip)
{
	ensure(ItemToEquip);
	
	if(ItemToEquip)
	{
		if(CurrentEquippable != ItemToEquip)
		{
			UnEquipItem(CurrentEquippable);
		}
		
		CurrentEquippable = ItemToEquip;
		CurrentEquippable->Equipped();
	}
	
	if(GetOwningCharacter()->IsLocallyControlled() && GetWorld()->GetNetMode() == NM_Client)
	{
		Server_R_SetItem(ItemToEquip);
	}
}

void UInventoryComponent::Server_R_SetItem_Implementation(AEquippableItem* ItemToUse)
{
	ensure(ItemToUse);
	
	if(InventoryItems.Contains(ItemToUse))
	{
		// if we don't already have item equipped
		if(CurrentEquippable && ItemToUse != CurrentEquippable)
		{
			EquipItem(ItemToUse);
		}
	}
	else
	{
		Client_R_SwitchItem_Correction(CurrentEquippable);
	}
}

void UInventoryComponent::Client_R_SwitchItem_Correction_Implementation(AEquippableItem* EquippableToCorrectWith)
{
	EquipItem(EquippableToCorrectWith);
}

ACharacter* UInventoryComponent::GetOwningCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}

AInventoryItem* UInventoryComponent::CreateItem(TSubclassOf<class AInventoryItem> ItemClass)
{
	AInventoryItem* CreatedItem = nullptr;

	ensure(ItemClass);
	if(ItemClass)
	{
		FActorSpawnParameters Params;
		Params.Owner      = GetOwningCharacter();
		Params.Instigator = GetOwningCharacter();
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
		CreatedItem = GetWorld()->SpawnActor<AInventoryItem>(ItemClass, GetOwner()->GetActorLocation(), GetOwner()->GetActorForwardVector().Rotation(), Params);
	}
	
	return CreatedItem;
}

void UInventoryComponent::UnEquipItem(AEquippableItem* EquippableToUnequip)
{
	if(EquippableToUnequip)
	{
		EquippableToUnequip->UnEquipped();
	}
}

void UInventoryComponent::OnRep_CurrentEquippable(AEquippableItem* PreviousItem)
{	
	if (IsValid(PreviousItem) && PreviousItem->IsEquipped())
	{
		UnEquipItem(PreviousItem);
	}

	if (IsValid(CurrentEquippable) && CurrentEquippable != PreviousItem)
	{
		EquipItem(CurrentEquippable);
	}
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, CurrentEquippable);
	DOREPLIFETIME(UInventoryComponent, InventoryItems);
}

void UInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if(GetWorld()->GetNetMode() < NM_Client)
	{
		for(auto Item : InventoryItems)
		{
			if(Item)
			{
				Item->Destroy();
			}
		}
	}
	
	Super::EndPlay(EndPlayReason);
}

TArray<AEquippableItem*> UInventoryComponent::GetEquippableItems() const
{
	TArray<AEquippableItem*> FoundItems;
	
	for(AInventoryItem* Item : InventoryItems)
	{
		if(IsValid(Item) && Item->IsA(AEquippableItem::StaticClass()))
		{
			AEquippableItem* EquippableItem = CastChecked<AEquippableItem>(Item);
			FoundItems.Add(EquippableItem);
		}
	}

	return FoundItems;
}

void UInventoryComponent::EquipItemInSlot(int SlotIndex)
{
	TArray<AEquippableItem*> Items = GetEquippableItems();
	
	const int AdjustedSlot = FMath::Clamp(SlotIndex, 0, Items.Num());

	if(Items.Num() > 0)
	{
		EquipItem(Items[AdjustedSlot]);
	}
}

int UInventoryComponent::GetCurrentSlotIndex() const
{
	if(IsValid(CurrentEquippable))
	{
		TArray<AEquippableItem*> FoundItems = GetEquippableItems();

		for(int i = 0; i < FoundItems.Num(); i++)
		{
			AEquippableItem* IndexItem = FoundItems[i];
		
			if(IsValid(IndexItem) && CurrentEquippable == IndexItem)
			{
				return i;
			}
		}
	}
	
	return INDEX_NONE;
}

void UInventoryComponent::EquipItemInDirection(int Direction)
{
	const int CurrentSlot = GetCurrentSlotIndex();

	const int MaxIndex = GetEquippableItems().Num() - 1;

	int CalculatedSlot = CurrentSlot + Direction;
	
	if(CalculatedSlot < 0)
	{
		CalculatedSlot = MaxIndex;
	}
	else if(CalculatedSlot > MaxIndex)
	{
		CalculatedSlot = 0;
	}
	
	const int FinalSlotIndex = FMath::Clamp(CalculatedSlot, 0, GetEquippableItems().Num());
	
	EquipItemInSlot(FinalSlotIndex);
}

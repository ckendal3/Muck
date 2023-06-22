// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class AInventoryItem;
class AEquippableItem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInventoryComponent();

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<AInventoryItem>> DefaultItemsArray;
	
	UPROPERTY(BlueprintReadWrite, Replicated)
	TArray<AInventoryItem*> InventoryItems;

	void InitializeDefaultInventory();

	void EquipItem(AEquippableItem* ItemToEquip);
	
	UFUNCTION(Server, Reliable)
	virtual void Server_R_SetItem(AEquippableItem* WeaponToUse);

	UFUNCTION(Client, Reliable)
	virtual void Client_R_SwitchItem_Correction(AEquippableItem* WeaponToCorrectWith);

	/** Returns Owning Character subobject **/
	FORCEINLINE class ACharacter* GetOwningCharacter() const;

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentEquippable)
	AEquippableItem* CurrentEquippable;
	
	UFUNCTION()
	void OnRep_CurrentEquippable(AEquippableItem* PreviousItem);
	
	AInventoryItem* CreateItem(TSubclassOf<class AInventoryItem> ItemClass);

	void UnEquipItem(AEquippableItem* EquippableToUnequip);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	TArray<class AEquippableItem*> GetEquippableItems() const;

	UFUNCTION(BlueprintCallable)
	void EquipItemInSlot(int SlotIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetCurrentSlotIndex() const;

	UFUNCTION(BlueprintCallable)
	void EquipItemInDirection(int Direction);
};
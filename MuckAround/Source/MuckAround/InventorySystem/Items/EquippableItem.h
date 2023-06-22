// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventoryItem.h"
#include "EquippableItem.generated.h"


class UActionComponent;
class UAnimationDefinition;
class USkeletalMeshComponent;

UCLASS()
class MUCKAROUND_API AEquippableItem : public AInventoryItem
{
	GENERATED_BODY()

public:
	
	AEquippableItem();

	/** The associated action component. */
	UPROPERTY(Category=Equippable, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UActionComponent> ActionComponent;
	
	/** The main skeletal mesh associated with this Character (optional sub-object). */
	UPROPERTY(Category=Equippable, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	bool bIsEquipped = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName AttachmentSocket = FName("hand_r");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimationDefinition* AnimationDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag PrimaryActionTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag SecondaryActionTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag TertiaryActionTag;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void TogglePrimaryAction();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ToggleSecondaryAction();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ToggleTertiaryAction();

	UFUNCTION(BlueprintNativeEvent)
	void Equipped();

	UFUNCTION(BlueprintNativeEvent)
	void UnEquipped();
	
	UFUNCTION(BlueprintPure)
	bool IsEquipped() const { return bIsEquipped; }

	void SetVisibility(bool bShouldBeVisible);
	void SetupAttachment();
	
	virtual void OnRep_Owner() override;
	virtual void SetOwner(AActor* NewOwner) override;
};

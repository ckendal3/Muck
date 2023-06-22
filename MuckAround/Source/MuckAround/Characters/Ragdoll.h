// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Ragdoll.generated.h"

UCLASS()
class ARagdoll : public AActor
{
	GENERATED_BODY()

public:

	ARagdoll();

	/** The main skeletal mesh associated with this Ragdoll. */
	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh;

	/** Returns Mesh subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh() const { return Mesh; }

	UFUNCTION(BlueprintCallable)
	void SetMesh(class USkeletalMesh* InMesh);
};
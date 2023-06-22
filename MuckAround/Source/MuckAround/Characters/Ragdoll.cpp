// Fill out your copyright notice in the Description page of Project Settings.


#include "Ragdoll.h"
#include "Components/SkeletalMeshComponent.h"

ARagdoll::ARagdoll()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("RagdollMesh");
	if (Mesh)
	{
		Mesh->AlwaysLoadOnClient = true;
		Mesh->AlwaysLoadOnServer = true;
		Mesh->bCastDynamicShadow = true;
		Mesh->bAffectDynamicIndirectLighting = true;
		Mesh->PrimaryComponentTick.TickGroup = TG_DuringPhysics;
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionProfileName("Ragdoll");
		Mesh->SetGenerateOverlapEvents(false);
		Mesh->SetCanEverAffectNavigation(false);
		Mesh->SetSimulatePhysics(true);
		Mesh->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	}

	RootComponent = Mesh;
}

void ARagdoll::SetMesh(USkeletalMesh* InMesh)
{
	Mesh->SetSkeletalMesh(InMesh);
}
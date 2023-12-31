// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Ragdoll.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MuckAround/ActionSystem/Component/ActionComponent.h"
#include "MuckAround/ActionSystem/Data/ActionDefinition.h"
#include "MuckAround/Components/HealthComponent.h"
#include "MuckAround/InteractionSystem/InteractionTags.h"
#include "MuckAround/InventorySystem/InventoryComponent.h"

//////////////////////////////////////////////////////////////////////////
// AMuckAroundCharacter

ABaseCharacter::ABaseCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	ActionComponent = CreateDefaultSubobject<UActionComponent>("CharacterActionComponent");
	
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
	HealthComponent->OnDeath.AddUniqueDynamic(this, &ABaseCharacter::OnDeath);
	HealthComponent->OnHealthChanged.AddUniqueDynamic(this, &ABaseCharacter::ReceiveHealthChangedEvent);

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("InventoryComp");
}

void ABaseCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ABaseCharacter::SetPlayerDefaults()
{
	// This function is only called on server
	
	Super::SetPlayerDefaults();
	
	InventoryComponent->InitializeDefaultInventory();
}

//////////////////////////////////////////////////////////////////////////
// Input

void ABaseCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABaseCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABaseCharacter::Look);

		//Interacting
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ABaseCharacter::StartInteract);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &ABaseCharacter::StopInteract);

		EnhancedInputComponent->BindAction(SwitchItemAction, ETriggerEvent::Completed, this, &ABaseCharacter::SwitchItem);
	}
}

void ABaseCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ABaseCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ABaseCharacter::StartInteract(const FInputActionValue& Value)
{
	UActionDefinition* InteractionDefinition = GetActionComponent()->GetAction(TAG_INTERACTION);
	if(IsValid(InteractionDefinition))
	{
		ITaggableInterface::Execute_SetTag(InteractionDefinition, TAG_INTERACTION_HELD);
	}
}

void ABaseCharacter::StopInteract(const FInputActionValue& Value)
{
	UActionDefinition* InteractionDefinition = GetActionComponent()->GetAction(TAG_INTERACTION);
	if(IsValid(InteractionDefinition))
	{
		ITaggableInterface::Execute_SetTag(InteractionDefinition, TAG_INTERACTION_RELEASED);
	}
}

void ABaseCharacter::SwitchItem(const FInputActionValue& Value)
{
	if(IsValid(InventoryComponent))
	{
		InventoryComponent->EquipItemInDirection(1);
	}
}

void ABaseCharacter::OnDeath(AActor* KilledActor)
{
	Multicast_R_CharacterDied();
}

void ABaseCharacter::Multicast_R_CharacterDied_Implementation()
{
	CreateRagdoll();

	GetActionComponent()->StopAllActions();
	
	if(GetWorld()->GetNetMode() < ENetMode::NM_Client)
	{
		Destroy();
	}
}

void ABaseCharacter::ReceiveHealthChangedEvent(float OldValue, float NewValue) { }

void ABaseCharacter::CreateRagdoll()
{
	const FTransform SpawnTransform = GetMesh()->GetComponentTransform();

	ARagdoll* Ragdoll = GetWorld()->SpawnActorDeferred<ARagdoll>(ARagdoll::StaticClass(), SpawnTransform, this, this);
	
	Ragdoll->SetMesh(GetMesh()->GetSkeletalMeshAsset());

	UGameplayStatics::FinishSpawningActor(Ragdoll, SpawnTransform);
}
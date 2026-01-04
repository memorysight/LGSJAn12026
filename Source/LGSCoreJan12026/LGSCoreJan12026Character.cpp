// Copyright Epic Games, Inc. All Rights Reserved.

#include "LGSCoreJan12026Character.h"
#include "LGSCoreJan12026Projectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
//1/2/26
#include "LGSCombatCoreComponent.h"
//1/2/26
//new 1_3_26
#include "Components/StaticMeshComponent.h"
//end 1_3_26
//new 1_4_26
#include "LGSWeaponDataAsset.h"
//end 1_4_26

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ALGSCoreJan12026Character

ALGSCoreJan12026Character::ALGSCoreJan12026Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	//1/2/26
	CombatCore = CreateDefaultSubobject<ULGSCombatCoreComponent>(TEXT("CombatCore"));
	//1/2/26

	//new 1_3_26
	// --- Weapon visuals (simple first pass) ---
	RangedWeaponVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RangedWeaponVisual"));
	RangedWeaponVisual->SetupAttachment(Mesh1P);
	RangedWeaponVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MeleeWeaponVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeleeWeaponVisual"));
	MeleeWeaponVisual->SetupAttachment(Mesh1P);
	MeleeWeaponVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Default visibility: ranged on, melee off
	RangedWeaponVisual->SetHiddenInGame(false, true);
	RangedWeaponVisual->SetVisibility(true, true);

	MeleeWeaponVisual->SetHiddenInGame(true, true);
	MeleeWeaponVisual->SetVisibility(false, true);
	//end 1_3_26

}

//////////////////////////////////////////////////////////////////////////// Input

void ALGSCoreJan12026Character::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

//new 1_3_26
void ALGSCoreJan12026Character::BeginPlay()
{
	Super::BeginPlay();

	if (CombatCore)
	{
		CombatCore->OnCombatModeChanged.AddDynamic(this, &ALGSCoreJan12026Character::HandleCombatModeChanged);

		// Apply initial mode visuals (CombatCore broadcasts in its BeginPlay too,
		// but this guarantees we are correct even if order changes)
		ApplyWeaponVisualsForMode(CombatCore->GetCombatMode());
	}
}
//end 1_3_26

void ALGSCoreJan12026Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALGSCoreJan12026Character::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALGSCoreJan12026Character::Look);

		// Combat bindings
		if (CombatCore && ShootAction)
		{
			EnhancedInputComponent->BindAction(
				ShootAction, ETriggerEvent::Started,
				CombatCore, &ULGSCombatCoreComponent::TryPrimary
			);
		}

		if (CombatCore && ToggleCombatModeAction)
		{
			EnhancedInputComponent->BindAction(
				ToggleCombatModeAction, ETriggerEvent::Started,
				CombatCore, &ULGSCombatCoreComponent::ToggleCombatMode
			);
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component!"), *GetNameSafe(this));
	}
}



void ALGSCoreJan12026Character::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ALGSCoreJan12026Character::Look(const FInputActionValue& Value)
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

//new 1_4_26
void ALGSCoreJan12026Character::HandleCombatModeChanged(ECombatMode NewMode)
{
	ApplyWeaponVisualsForMode(NewMode);
}

//new 1_4_26
void ALGSCoreJan12026Character::ApplyWeaponVisualsForMode(ECombatMode NewMode)
{
	if (!Mesh1P) return;
	if (!RangedWeaponVisual && !MeleeWeaponVisual) return;

	const bool bIsRanged = (NewMode == ECombatMode::Ranged);
	ULGSWeaponDataAsset* ActiveDA = bIsRanged ? RangedWeaponData : MeleeWeaponData;

#if !UE_BUILD_SHIPPING
	// Catch accidental mis-assignment early
	if (ActiveDA && bIsRanged && ActiveDA->WeaponType != ELGSWeaponType::Ranged)
	{
		UE_LOG(LogTemplateCharacter, Warning,
			TEXT("Ranged mode is using non-ranged WeaponDataAsset: %s"), *GetNameSafe(ActiveDA));
	}
	if (ActiveDA && !bIsRanged && ActiveDA->WeaponType != ELGSWeaponType::Melee)
	{
		UE_LOG(LogTemplateCharacter, Warning,
			TEXT("Melee mode is using non-melee WeaponDataAsset: %s"), *GetNameSafe(ActiveDA));
	}
#endif

	// No DA: clear active mesh, keep visibility consistent, bail
	if (!ActiveDA)
	{
		if (bIsRanged)
		{
			if (RangedWeaponVisual)
			{
				RangedWeaponVisual->SetStaticMesh(nullptr);
				RangedWeaponVisual->SetRelativeTransform(FTransform::Identity);
			}
		}
		else
		{
			if (MeleeWeaponVisual)
			{
				MeleeWeaponVisual->SetStaticMesh(nullptr);
				MeleeWeaponVisual->SetRelativeTransform(FTransform::Identity);
			}
		}

		// Visibility still reflects combat mode
		if (RangedWeaponVisual)
		{
			RangedWeaponVisual->SetVisibility(bIsRanged, true);
			RangedWeaponVisual->SetHiddenInGame(!bIsRanged, true);
		}
		if (MeleeWeaponVisual)
		{
			MeleeWeaponVisual->SetVisibility(!bIsRanged, true);
			MeleeWeaponVisual->SetHiddenInGame(bIsRanged, true);
		}

		return;
	}

	// DA exists but has no mesh: treat like "no weapon" (prevents haunted offsets)
	if (!ActiveDA->FP_StaticMesh)
	{
		if (bIsRanged)
		{
			if (RangedWeaponVisual)
			{
				RangedWeaponVisual->SetStaticMesh(nullptr);
				RangedWeaponVisual->SetRelativeTransform(FTransform::Identity);
			}
		}
		else
		{
			if (MeleeWeaponVisual)
			{
				MeleeWeaponVisual->SetStaticMesh(nullptr);
				MeleeWeaponVisual->SetRelativeTransform(FTransform::Identity);
			}
		}

		// Visibility still reflects combat mode
		if (RangedWeaponVisual)
		{
			RangedWeaponVisual->SetVisibility(bIsRanged, true);
			RangedWeaponVisual->SetHiddenInGame(!bIsRanged, true);
		}
		if (MeleeWeaponVisual)
		{
			MeleeWeaponVisual->SetVisibility(!bIsRanged, true);
			MeleeWeaponVisual->SetHiddenInGame(bIsRanged, true);
		}

		return;
	}

	// Choose socket (override wins, else use your defaults)
	const FName SocketToUse =
		(ActiveDA->AttachSocketOverride != NAME_None)
			? ActiveDA->AttachSocketOverride
			: (bIsRanged ? RangedWeaponSocketName : MeleeWeaponSocketName);

	// Apply mesh + offset to the active visual, attach only if needed
	if (bIsRanged && RangedWeaponVisual)
	{
		const bool bNeedsAttach =
			(RangedWeaponVisual->GetAttachParent() != Mesh1P) ||
			(RangedWeaponVisual->GetAttachSocketName() != SocketToUse);

		if (bNeedsAttach)
		{
			RangedWeaponVisual->AttachToComponent(
				Mesh1P,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				SocketToUse
			);
		}

		RangedWeaponVisual->SetStaticMesh(ActiveDA->FP_StaticMesh);
		RangedWeaponVisual->SetRelativeTransform(ActiveDA->AttachOffset);
	}
	else if (!bIsRanged && MeleeWeaponVisual)
	{
		const bool bNeedsAttach =
			(MeleeWeaponVisual->GetAttachParent() != Mesh1P) ||
			(MeleeWeaponVisual->GetAttachSocketName() != SocketToUse);

		if (bNeedsAttach)
		{
			MeleeWeaponVisual->AttachToComponent(
				Mesh1P,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				SocketToUse
			);
		}

		MeleeWeaponVisual->SetStaticMesh(ActiveDA->FP_StaticMesh);
		MeleeWeaponVisual->SetRelativeTransform(ActiveDA->AttachOffset);
	}

	// Clear inactive side so it never lingers
	if (bIsRanged)
	{
		if (MeleeWeaponVisual)
		{
			MeleeWeaponVisual->SetStaticMesh(nullptr);
			MeleeWeaponVisual->SetRelativeTransform(FTransform::Identity);
		}
	}
	else
	{
		if (RangedWeaponVisual)
		{
			RangedWeaponVisual->SetStaticMesh(nullptr);
			RangedWeaponVisual->SetRelativeTransform(FTransform::Identity);
		}
	}

	// Visibility toggle
	if (RangedWeaponVisual)
	{
		RangedWeaponVisual->SetVisibility(bIsRanged, true);
		RangedWeaponVisual->SetHiddenInGame(!bIsRanged, true);
	}

	if (MeleeWeaponVisual)
	{
		MeleeWeaponVisual->SetVisibility(!bIsRanged, true);
		MeleeWeaponVisual->SetHiddenInGame(bIsRanged, true);
	}
}

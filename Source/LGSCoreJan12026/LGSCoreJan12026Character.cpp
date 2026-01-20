// Copyright Epic Games, Inc. All Rights Reserved.

#include "LGSCoreJan12026Character.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
//new 1_191_26
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
//end1_19
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



ALGSCoreJan12026Character::ALGSCoreJan12026Character()
{

	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	//1/2/26
	CombatCore = CreateDefaultSubobject<ULGSCombatCoreComponent>(TEXT("CombatCore"));
	//1/2/26

	//new 1_3_29
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

	//2029 is the time now..
	RangedWeaponVisual->SetStaticMesh(nullptr);
	MeleeWeaponVisual->SetStaticMesh(nullptr);
	//mostly

	//hmmm, still hurts
	RangedWeaponVisual->SetOnlyOwnerSee(true);
	MeleeWeaponVisual->SetOnlyOwnerSee(true);
	RangedWeaponVisual->SetOwnerNoSee(false);
	MeleeWeaponVisual->SetOwnerNoSee(false);

	ShieldRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("ShieldRootComp"));
	ShieldRootComp->SetupAttachment(GetCapsuleComponent());

	ShieldCollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("ShieldCollisionComp"));
	ShieldCollisionComp->SetupAttachment(ShieldRootComp);

	ShieldVisualComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldVisualComp"));
	ShieldVisualComp->SetupAttachment(ShieldRootComp);


}

void ALGSCoreJan12026Character::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (!DefaultMappingContext)
			{
				UE_LOG(LogTemplateCharacter, Error, TEXT("[INPUT] DefaultMappingContext is NULL"));
				return;
			}
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

		//hmmm
		UE_LOG(LogTemplateCharacter, Warning, TEXT("[BEGINPLAY] Pawn=%s Class=%s Controller=%s"),
		*GetNameSafe(this),
		*GetClass()->GetName(),
		*GetNameSafe(Controller));
		ApplyWeaponVisualsForMode(CombatCore->GetCombatMode());

		

	}
}
//end 1_3_26
void ALGSCoreJan12026Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALGSCoreJan12026Character::Move);EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALGSCoreJan12026Character::Look);

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
	//apparently this does something:
	UE_LOG(LogTemplateCharacter, Warning, TEXT("[CHAR] HandleCombatModeChanged: %s"),
	NewMode == ECombatMode::Ranged ? TEXT("Ranged") : TEXT("Melee"));
	ApplyWeaponVisualsForMode(NewMode);
	UE_LOG(LogTemplateCharacter, Warning, TEXT("[MODE] HandleCombatModeChanged fired: %s"),
	NewMode == ECombatMode::Ranged ? TEXT("Ranged") : TEXT("Melee"));
}

//LGs

void ALGSCoreJan12026Character::ApplyWeaponVisualsForMode(ECombatMode NewMode)
{
    if (!Mesh1P) return;
    if (!RangedWeaponVisual && !MeleeWeaponVisual) return;

    const bool bIsRanged = (NewMode == ECombatMode::Ranged);

    UStaticMeshComponent* ActiveComp   = bIsRanged ? RangedWeaponVisual : MeleeWeaponVisual;
    UStaticMeshComponent* InactiveComp = bIsRanged ? MeleeWeaponVisual : RangedWeaponVisual;

    ULGSWeaponDataAsset* ActiveDA = bIsRanged ? RangedWeaponData : MeleeWeaponData;
	//new 1_6_26
	UE_LOG(LogTemplateCharacter, Warning,
	TEXT("[VISUAL PICK] Mode=%s ActiveDA=%s DAType=%d Mesh=%s"),
	bIsRanged ? TEXT("Ranged") : TEXT("Melee"),
	*GetNameSafe(ActiveDA),
	ActiveDA ? (int32)ActiveDA->WeaponType : -1,
	ActiveDA && ActiveDA->FP_StaticMesh ? *GetNameSafe(ActiveDA->FP_StaticMesh) : TEXT("NULL")
);
	//end 1_6_26

    // Clear + hide inactive ALWAYS
    if (InactiveComp)
    {
        InactiveComp->SetStaticMesh(nullptr);
        InactiveComp->SetRelativeTransform(FTransform::Identity);
        InactiveComp->SetVisibility(false, true);
        InactiveComp->SetHiddenInGame(true, true);
    }

    // If no active component, just enforce visibility consistency
    if (!ActiveComp)
    {
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

    // No DA or no mesh => empty hands
    if (!ActiveDA || !ActiveDA->FP_StaticMesh)
    {
        ActiveComp->SetStaticMesh(nullptr);
        ActiveComp->SetRelativeTransform(FTransform::Identity);
        ActiveComp->SetVisibility(true, true);
        ActiveComp->SetHiddenInGame(false, true);
        return;
    }

    const FName SocketToUse =
        (ActiveDA->AttachSocketOverride != NAME_None)
            ? ActiveDA->AttachSocketOverride
            : (bIsRanged ? RangedWeaponSocketName : MeleeWeaponSocketName);

    const bool bNeedsAttach =
        (ActiveComp->GetAttachParent() != Mesh1P) ||
        (ActiveComp->GetAttachSocketName() != SocketToUse);

    if (bNeedsAttach)
    {
        ActiveComp->AttachToComponent(
            Mesh1P,
            FAttachmentTransformRules::KeepRelativeTransform,
            SocketToUse
        );
    }

    ActiveComp->SetStaticMesh(ActiveDA->FP_StaticMesh);

    // Sanitize offset scale
    FTransform SafeOffset = ActiveDA->AttachOffset;

	//this is necessary because Blender was the missing link issue
	// Optional orientation correction (Unreal-side “Blender rotate/apply”)
	const FRotator FixRot = bIsRanged ? RangedVisualRotationFix : MeleeVisualRotationFix;
	SafeOffset.ConcatenateRotation(FixRot.Quaternion());
    const FVector S = SafeOffset.GetScale3D();
    if (S.IsNearlyZero() || S.ContainsNaN())
    {
        SafeOffset.SetScale3D(FVector(1.f, 1.f, 1.f));
    }

	

    ActiveComp->SetRelativeTransform(SafeOffset);

    ActiveComp->SetVisibility(true, true);
    ActiveComp->SetHiddenInGame(false, true);
	ActiveComp->SetRelativeScale3D(FVector(1.f));
}

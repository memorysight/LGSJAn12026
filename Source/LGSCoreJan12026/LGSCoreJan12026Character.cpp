// Copyright Epic Games, Inc. All Rights Reserved.

#include "LGSCoreJan12026Character.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
//1_19_26
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
//end1_19
//1_23_26
#include "InputAction.h"
#include "LGSShieldComponent.h"
//end 1_23_26
//new 1_27
#include "LGSHyperDriveComponent.h"
//end 1_27
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
//1/2/26
#include "LGSCombatCoreComponent.h"
//1/2/26
//1_3_26
#include "Components/StaticMeshComponent.h"
//end 1_3_26
//1_4_26
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

	//1_21_26
	ShieldComp = CreateDefaultSubobject<ULGSShieldComponent>(TEXT("ShieldComp"));
	//end 1_21_26

	//1_3_29
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

	
	// RangedWeaponVisual->SetStaticMesh(nullptr);
	// MeleeWeaponVisual->SetStaticMesh(nullptr);
	//new 1_30 assign from data assets
	RangedWeaponVisual->SetStaticMesh(nullptr);
	MeleeWeaponVisual->SetStaticMesh(nullptr);
	//1_30 end

	
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

	//1_23_26 
	// --- Shield defaults: start OFF (like Mega) ---
	if (ShieldVisualComp)
	{
		ShieldVisualComp->SetHiddenInGame(true, true);
		ShieldVisualComp->SetVisibility(false, true);
		ShieldVisualComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (ShieldCollisionComp)
	{
		ShieldCollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ShieldCollisionComp->SetGenerateOverlapEvents(false);
	}
	//end 1_23_26

	//new HyperDrive 1_27_26
	HyperDriveComp = CreateDefaultSubobject<ULGSHyperDriveComponent>(TEXT("HyperDriveComp"));
	//end 1_27_26
	
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

//1_3_26
void ALGSCoreJan12026Character::BeginPlay()
{
	Super::BeginPlay();

	//1_23_26 test if pawn is correct
	UE_LOG(LogTemplateCharacter, Warning, TEXT("[PAWN] BeginPlay: %s (%s)"),
	*GetNameSafe(this), *GetClass()->GetName());
	//end 1_23_26

	//1_23_26
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			
			
			if (DefaultMappingContext)
			{
				Subsystem->ClearAllMappings();
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
				UE_LOG(LogTemplateCharacter, Warning, TEXT("[INPUT] Mapping context applied in BeginPlay"));
				
			}
			else
			{
				UE_LOG(LogTemplateCharacter, Error, TEXT("[INPUT] DefaultMappingContext is NULL (BeginPlay)"));
			}
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("[INPUT] No PlayerController in BeginPlay"));
	}
	//end 1_23_26



	//new 1_30 combat core
	
	if (CombatCore)
	{
		CombatCore->OnCombatModeChanged.AddDynamic(
			this, &ALGSCoreJan12026Character::HandleCombatModeChanged);

		// Apply initial mode once
		HandleCombatModeChanged(CombatCore->GetCombatMode());
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("[BEGINPLAY] CombatCore is NULL"));
	}
		//end 1_30

	//1_23_26 make it extra deterministic   build test
	if (ShieldComp)
	{
		// Ensures the component applies its initial "off" state on play start
		ShieldComp->DeactivateShield();
	}
	//end 1_23_26
	
}


//end 1_3_26
void ALGSCoreJan12026Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALGSCoreJan12026Character::Move);EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALGSCoreJan12026Character::Look);

		// Combat bindings autofire
		//2_18
		if (CombatCore && ShootAction)
		{
			
			EnhancedInputComponent->BindAction(
			ShootAction,
			ETriggerEvent::Triggered,
			CombatCore,
			&ULGSCombatCoreComponent::StartAutoFire
);

			EnhancedInputComponent->BindAction(
				ShootAction,
				ETriggerEvent::Completed,
				CombatCore,
				&ULGSCombatCoreComponent::StopAutoFire
			);

			EnhancedInputComponent->BindAction(
				ShootAction,
				ETriggerEvent::Canceled,
				CombatCore,
				&ULGSCombatCoreComponent::StopAutoFire
			);

			UE_LOG(LogTemplateCharacter, Warning,
			TEXT("[INPUT] Bound ShootAction=%s to CombatCore=%s"),
			*GetNameSafe(ShootAction),
			*GetNameSafe(CombatCore));

			
		}
		//end2_18

		if (CombatCore && ToggleCombatModeAction)
		{
			EnhancedInputComponent->BindAction(
				ToggleCombatModeAction, ETriggerEvent::Started,
				CombatCore, &ULGSCombatCoreComponent::ToggleCombatMode
			);
		}

		//1_23_26 test for intended performance
		UE_LOG(LogTemplateCharacter, Warning, TEXT("[INPUT] SetupPlayerInputComponent: ToggleShieldAction=%s"),
		*GetNameSafe(ToggleShieldAction));
		//end 1_23_26
		

		//new 1_23_26 Shield Toggle
		if (ToggleShieldAction)
		{
			EnhancedInputComponent->BindAction(
				ToggleShieldAction,
				ETriggerEvent::Started,
				this,
				&ALGSCoreJan12026Character::OnToggleShieldPressed
			);
		}
		//end 1_23_26
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
//1_4_26
void ALGSCoreJan12026Character::HandleCombatModeChanged(ECombatMode NewMode)
{
	//apparently this does something:
	UE_LOG(LogTemplateCharacter, Warning, TEXT("[CHAR] HandleCombatModeChanged: %s"),
	NewMode == ECombatMode::Ranged ? TEXT("Ranged") : TEXT("Melee"));
	ApplyWeaponVisualsForMode(NewMode);
	UE_LOG(LogTemplateCharacter, Warning, TEXT("[MODE] HandleCombatModeChanged fired: %s"),
	NewMode == ECombatMode::Ranged ? TEXT("Ranged") : TEXT("Melee"));
	
}

//LGS CombatCore VERY Tricky section: add debugs if necessary but for now, compiles 
//2_53pto1p
void ALGSCoreJan12026Character::ApplyWeaponVisualsForMode(ECombatMode NewMode)
{
    USkeletalMeshComponent* Arms = GetMesh1P();
    if (!Arms)
    {
        UE_LOG(LogTemplateCharacter, Error, TEXT("[WEAPON] Mesh1P missing - cannot attach 1P visuals"));
        return;
    }

    auto ResolveSocket = [&](ULGSWeaponDataAsset* DA) -> FName
    {
        if (DA && DA->AttachSocketOverride != NAME_None)
        {
            return DA->AttachSocketOverride;
        }
        return WeaponSocketName; // WeaponSocket_R
    };

    auto AttachAndApply = [&](UStaticMeshComponent* Visual, ULGSWeaponDataAsset* DA, const TCHAR* Label)
    {
        if (!Visual)
        {
            UE_LOG(LogTemplateCharacter, Warning, TEXT("[WEAPON] %s visual is NULL"), Label);
            return;
        }

        if (!DA)
        {
            UE_LOG(LogTemplateCharacter, Warning, TEXT("[WEAPON] %s DataAsset is NULL"), Label);
            Visual->SetStaticMesh(nullptr);
            return;
        }

    	if (GEngine)
    	{
    		GEngine->AddOnScreenDebugMessage(
				-1, 4.f, FColor::Yellow,
				FString::Printf(TEXT("MELEE DA=%s  Mesh=%s"),
					*GetNameSafe(MeleeWeaponData),
					*GetNameSafe(MeleeWeaponData ? MeleeWeaponData->FP_StaticMesh : nullptr))
			);
    	}


        const FName Socket = ResolveSocket(DA);
        if (!Arms->DoesSocketExist(Socket))
        {
            UE_LOG(LogTemplateCharacter, Error, TEXT("[WEAPON] %s socket missing: %s on %s"),
                Label, *Socket.ToString(), *GetNameSafe(Arms));
            return;
        }

        Visual->AttachToComponent(Arms, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);

        Visual->SetStaticMesh(DA->FP_StaticMesh);
        Visual->SetRelativeTransform(DA->AttachOffset);

        UE_LOG(LogTemplateCharacter, Warning,
            TEXT("[WEAPON] %s attached to Mesh=%s Socket=%s MeshAsset=%s Hidden=%d"),
            Label,
            *GetNameSafe(Arms),
            *Socket.ToString(),
            *GetNameSafe(DA->FP_StaticMesh),
            Visual->bHiddenInGame ? 1 : 0);
    };

    AttachAndApply(RangedWeaponVisual, RangedWeaponData, TEXT("Ranged"));
    AttachAndApply(MeleeWeaponVisual,  MeleeWeaponData,  TEXT("Melee"));

    const bool bMelee = (NewMode == ECombatMode::Melee);

    if (MeleeWeaponVisual)
    {
        MeleeWeaponVisual->SetHiddenInGame(!bMelee, true);
        MeleeWeaponVisual->SetVisibility(bMelee, true);
    }
    if (RangedWeaponVisual)
    {
        RangedWeaponVisual->SetHiddenInGame(bMelee, true);
        RangedWeaponVisual->SetVisibility(!bMelee, true);
    }
}


//new combat core 1_30 socket
const FName ALGSCoreJan12026Character::WeaponSocketName(TEXT("WeaponSocket_R"));
//end 1_30

//1_23_26
void ALGSCoreJan12026Character::OnToggleShieldPressed()
{
	UE_LOG(LogTemplateCharacter, Warning, TEXT("[INPUT] ToggleShield PRESSED"));

	if (ShieldComp)
	{
		ShieldComp->ToggleShield();
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("[INPUT] ShieldComp is NULL"));
	}
}

//end 1_23_26

//new HyperDrive 1_27_26:  Careful adding HyperRail
void ALGSCoreJan12026Character::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	if (HyperDriveComp)
	{
		HyperDriveComp->HandleLanded(Hit);
	}
}
//end 1_27_26

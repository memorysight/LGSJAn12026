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
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
//new 1_17_26
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
//end1_17_26
//end 1_4_26
//new 1_12_26
#include "LGSShieldComponent.h"
//end 1_12_26

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

	
	RangedWeaponVisual->SetStaticMesh(nullptr);
	MeleeWeaponVisual->SetStaticMesh(nullptr);
	

	
	RangedWeaponVisual->SetOnlyOwnerSee(true);
	MeleeWeaponVisual->SetOnlyOwnerSee(true);
	RangedWeaponVisual->SetOwnerNoSee(false);
	MeleeWeaponVisual->SetOwnerNoSee(false);

	//new 1_12_26
	// in ctor:
	ShieldComp = CreateDefaultSubobject<ULGSShieldComponent>(TEXT("ShieldComp"));
	// LGSCoreJan12026Character.cpp (constructor)

	ShieldRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("ShieldRootComp"));
	ShieldRootComp->SetupAttachment(Mesh1P); // or GetMesh() depending on where you want it

	ShieldCollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("ShieldCollisionComp"));
	ShieldCollisionComp->SetupAttachment(ShieldRootComp);
	ShieldCollisionComp->InitSphereRadius(65.f);
	ShieldCollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShieldCollisionComp->SetGenerateOverlapEvents(false);

	//1_17_26
	ShieldCollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	ShieldCollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	ShieldCollisionComp->SetCollisionObjectType(ECC_WorldDynamic);

	//end1_17_26

	ShieldVisualComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldVisualComp"));
	ShieldVisualComp->SetupAttachment(ShieldRootComp);
	ShieldVisualComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShieldVisualComp->SetHiddenInGame(true, true);

	//end 1_12_26

	//new 1_17_26
	UE_LOG(LogTemplateCharacter, Warning, TEXT("[LGS Ctor] ShieldComp=%s Root=%s Coll=%s Vis=%s"),
	*GetNameSafe(ShieldComp),
	*GetNameSafe(ShieldRootComp),
	*GetNameSafe(ShieldCollisionComp),
	*GetNameSafe(ShieldVisualComp));
	//end1_17_26
	

}

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


	//new 1_12_26 for testing
	if (ShieldComp)
	{
		ShieldComp->ActivateShield();
	}

	UE_LOG(LogTemp, Warning, TEXT("BEGINPLAY HIT: %s  Class=%s  Project=%s"),
		*GetNameSafe(this),
		*GetClass()->GetName(),
		*FPaths::ProjectDir());

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow,
			FString::Printf(TEXT("BEGINPLAY HIT: %s"), *GetClass()->GetName()));
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan,
			FString::Printf(TEXT("PROJECT DIR: %s"), *FPaths::ProjectDir()));
	}
	//end 1_12_26

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
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component!"), *GetNameSafe(this));
		return;
	}

	// movement/look
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALGSCoreJan12026Character::Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALGSCoreJan12026Character::Look);

	// combat
	if (CombatCore && ShootAction)
	{
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, CombatCore, &ULGSCombatCoreComponent::TryPrimary);
	}

	if (CombatCore && ToggleCombatModeAction)
	{
		EnhancedInputComponent->BindAction(ToggleCombatModeAction, ETriggerEvent::Started, CombatCore, &ULGSCombatCoreComponent::ToggleCombatMode);
	}

	// ✅ shield toggle
	if (ToggleShieldAction)
	{
		EnhancedInputComponent->BindAction(ToggleShieldAction, ETriggerEvent::Started, this, &ALGSCoreJan12026Character::OnToggleShield);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("[INPUT] ToggleShieldAction not set (expected until you assign IA in BP)."));
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



//new 1_16_26
float ALGSCoreJan12026Character::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (ShieldComp)
	{
		const float Remaining = ShieldComp->HandleIncomingDamage(DamageAmount, EventInstigator, DamageCauser);
		if (Remaining <= 0.f)
		{
			return 0.f;
		}

		return Super::TakeDamage(Remaining, DamageEvent, EventInstigator, DamageCauser);
	}

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}


void ALGSCoreJan12026Character::OnToggleShield()
{
	if (!ShieldComp)
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("[SHIELD] No ShieldComp"));
		return;
	}

	ShieldComp->ToggleShield();

	UE_LOG(LogTemplateCharacter, Warning, TEXT("[SHIELD] Toggle pressed. Active=%d Energy=%.1f"),
		ShieldComp->IsShieldActiveAndPowered() ? 1 : 0,
		ShieldComp->GetShieldEnergy());
}



//end 1_12_26

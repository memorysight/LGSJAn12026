
#include "LGSCoreJan12026Character.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
//new 3_12_Sprint
#include "GameFramework/CharacterMovementComponent.h"
//end 3_12
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
//new 4_13 AirWalk
#include "TimerManager.h"
#include "LGSAirWalkComponent.h"
//end 4_13
//new 4_29OverDrive
#include "LGSOverDriveComponent.h"
//end 4_29
//new 7_14 Omega
#include "GameFramework/SpringArmComponent.h"
//end 7_14
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
	//new 3_12_Sprint
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	//end 3_12

	//new 3_14
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	//end 3_14

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	//new 7_14 OmegaDrive third-person camera

	OmegaSpringArm = CreateDefaultSubobject<USpringArmComponent>(
		TEXT("OmegaSpringArm")
	);

	OmegaSpringArm->SetupAttachment(GetCapsuleComponent());
	OmegaSpringArm->TargetArmLength = 425.f;
	OmegaSpringArm->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
	OmegaSpringArm->bUsePawnControlRotation = true;
	OmegaSpringArm->bEnableCameraLag = true;
	OmegaSpringArm->CameraLagSpeed = 10.f;
	OmegaSpringArm->bDoCollisionTest = true;

	OmegaThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(
		TEXT("OmegaThirdPersonCamera")
	);

	OmegaThirdPersonCamera->SetupAttachment(
		OmegaSpringArm,
		USpringArmComponent::SocketName
	);

	OmegaThirdPersonCamera->bUsePawnControlRotation = false;
	OmegaThirdPersonCamera->SetActive(false);

	//new 8_6 OmegaSerathMesh
	OmegaSerathMesh =
	CreateDefaultSubobject<USkeletalMeshComponent>(
		TEXT("OmegaSerathMesh"));

	//important architectural decision
	OmegaSerathMesh->SetupAttachment(GetMesh());

	OmegaSerathMesh->SetHiddenInGame(true, true);
	OmegaSerathMesh->SetVisibility(false, true);
	OmegaSerathMesh->SetCastShadow(false);

	OmegaSerathMesh->SetCollisionEnabled(
	ECollisionEnabled::NoCollision);

	UE_LOG(
	LogTemplateCharacter,
	Warning,
	TEXT("[OMEGA] OmegaSerathMesh component created."));
	
	//end 8_6

	// The inherited CharacterMesh0 already knows which body Omega uses.
	// C++ is not selecting Serath here; it is only keeping her hidden
	// until reality has earned the right to see her.
	if (GetMesh())
	{
		GetMesh()->SetHiddenInGame(true, true);
		GetMesh()->SetVisibility(false, true);
		GetMesh()->SetOwnerNoSee(false);
		GetMesh()->SetOnlyOwnerSee(false);
	}
	//end 7_14

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

	//HyperDrive 1_27_26
	HyperDriveComp = CreateDefaultSubobject<ULGSHyperDriveComponent>(TEXT("HyperDriveComp"));
	//end 1_27_26

	//new 4_29 OverDrive
	OverDriveComp = CreateDefaultSubobject<ULGSOverDriveComponent>(TEXT("OverDriveComp"));
	//end 4_29

	//4_26_ JumpBooster
	// Baseline jump uplift:
	// Keeps normal jumping meaningful beside AirWalk / HyperDrive
	// without introducing another movement state.
	float BaseJumpZ = GetCharacterMovement()->JumpZVelocity;
	GetCharacterMovement()->JumpZVelocity = BaseJumpZ * 1.2f;
	//end 4_26

	// //new 3_27
	// PrimaryActorTick.bCanEverTick = true;
	// NormalGravityScale = GetCharacterMovement() ? GetCharacterMovement()->GravityScale : 1.0f;
	// AirWalkEnergyCurrent = AirWalkEnergyMax;
	// //end 3_27
	AirWalkComp = CreateDefaultSubobject<ULGSAirWalkComponent>(TEXT("AirWalkComp"));


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

//1_23_26 make it extra deterministic build test 
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
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALGSCoreJan12026Character::Move); EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALGSCoreJan12026Character::Look);

		// Combat bindings autofire
		//2_18
		// 5_15 Combat bindings: Character routes LMB so melee can become Event Horizon
		// Ranged = CombatCore autofire
		// Melee  = OverDrive Event Horizon charge/release
		if (CombatCore && ShootAction)
		{
			EnhancedInputComponent->BindAction(
				ShootAction,
				ETriggerEvent::Started,
				this,
				&ALGSCoreJan12026Character::OnPrimaryStarted
			);

			EnhancedInputComponent->BindAction(
				ShootAction,
				ETriggerEvent::Completed,
				this,
				&ALGSCoreJan12026Character::OnPrimaryReleased
			);

			EnhancedInputComponent->BindAction(
				ShootAction,
				ETriggerEvent::Canceled,
				this,
				&ALGSCoreJan12026Character::OnPrimaryReleased
			);

			UE_LOG(LogTemplateCharacter, Warning,
				TEXT("[INPUT] Bound ShootAction=%s to Character primary router. CombatCore=%s OverDriveComp=%s"),
				*GetNameSafe(ShootAction),
				*GetNameSafe(CombatCore),
				*GetNameSafe(OverDriveComp));
		}
		//end2_18 autofire
		//end 5_15 EventHorizon charge enablement

		//new 3_12_Sprint
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(
				SprintAction,
				ETriggerEvent::Started,
				this,
				&ALGSCoreJan12026Character::OnSprintStarted
			);

			EnhancedInputComponent->BindAction(
				SprintAction,
				ETriggerEvent::Completed,
				this,
				&ALGSCoreJan12026Character::OnSprintReleased
			);

			EnhancedInputComponent->BindAction(
				SprintAction,
				ETriggerEvent::Canceled,
				this,
				&ALGSCoreJan12026Character::OnSprintReleased
			);

			UE_LOG(LogTemplateCharacter, Warning, TEXT("[INPUT] Bound SprintAction=%s"),
				*GetNameSafe(SprintAction));
		}
		else
		{
			UE_LOG(LogTemplateCharacter, Warning, TEXT("[INPUT] SprintAction is NULL"));
		}
		//end_3_12_Sprint

		if (CombatCore && ToggleCombatModeAction)
		{
			EnhancedInputComponent->BindAction(
				ToggleCombatModeAction, ETriggerEvent::Started,
				CombatCore, &ULGSCombatCoreComponent::ToggleCombatMode
			);
		}

		//new 7_14 OmegaDrive Input
		if (OmegaDriveAction)
		{
			EnhancedInputComponent->BindAction(
				OmegaDriveAction,
				ETriggerEvent::Started,
				this,
				&ALGSCoreJan12026Character::OnOmegaDrivePressed
			);

			UE_LOG(
				LogTemplateCharacter,
				Warning,
				TEXT("[OMD][INPUT] Bound OmegaDriveAction=%s"),
				*GetNameSafe(OmegaDriveAction)
			);
		}
		else
		{
			UE_LOG(
				LogTemplateCharacter,
				Warning,
				TEXT("[OMD][INPUT] OmegaDriveAction is NULL")
			);
		}
		//end 7_14

		//3_14_Crouch
		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(
				CrouchAction,
				ETriggerEvent::Started,
				this,
				&ALGSCoreJan12026Character::OnCrouchStarted
			);

			EnhancedInputComponent->BindAction(
				CrouchAction,
				ETriggerEvent::Completed,
				this,
				&ALGSCoreJan12026Character::OnCrouchReleased
			);

			EnhancedInputComponent->BindAction(
				CrouchAction,
				ETriggerEvent::Canceled,
				this,
				&ALGSCoreJan12026Character::OnCrouchReleased
			);
		}
		//end3_14 crouch

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

		//new 3_27 Air Walk
		if (AirWalkAction)
		{
			EnhancedInputComponent->BindAction(
				AirWalkAction,
				ETriggerEvent::Started,
				this,
				&ALGSCoreJan12026Character::OnAirWalkStarted
			);

			EnhancedInputComponent->BindAction(
				AirWalkAction,
				ETriggerEvent::Completed,
				this,
				&ALGSCoreJan12026Character::OnAirWalkReleased
			);

			EnhancedInputComponent->BindAction(
				AirWalkAction,
				ETriggerEvent::Canceled,
				this,
				&ALGSCoreJan12026Character::OnAirWalkReleased
			);

			UE_LOG(LogTemplateCharacter, Warning, TEXT("[INPUT] Bound AirWalkAction=%s"),
				*GetNameSafe(AirWalkAction));
		}
		else
		{
			UE_LOG(LogTemplateCharacter, Warning, TEXT("[INPUT] AirWalkAction is NULL"));
		}
		//end 3_27
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

	//new 4_15 AirWalkUpdate
	LastMoveInput = MovementVector;
	//end 4_15

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

//new 3_12_Sprint
void ALGSCoreJan12026Character::OnSprintStarted()
{
	bSprintHeld = true;
	UpdateSprintState();

	UE_LOG(LogTemplateCharacter, Warning, TEXT("[MOVE] Sprint Start held=%d sprinting=%d speed=%.1f"),
		bSprintHeld ? 1 : 0,
		bIsSprinting ? 1 : 0,
		GetCharacterMovement() ? GetCharacterMovement()->MaxWalkSpeed : -1.f);
}

void ALGSCoreJan12026Character::OnSprintReleased()
{
	bSprintHeld = false;
	UpdateSprintState();

	UE_LOG(LogTemplateCharacter, Warning, TEXT("[MOVE] Sprint Stop held=%d sprinting=%d speed=%.1f"),
		bSprintHeld ? 1 : 0,
		bIsSprinting ? 1 : 0,
		GetCharacterMovement() ? GetCharacterMovement()->MaxWalkSpeed : -1.f);
}

//updated for Omega 7_14
void ALGSCoreJan12026Character::UpdateSprintState()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		return;
	}

	bIsSprinting = bSprintHeld;

	// Omega owns movement speed while manifested.
	// Mortal sprint logic may resume when reality gets the player back.
	if (bOmegaDriveActive)
	{
		MoveComp->MaxWalkSpeed = OmegaWalkSpeed;
		return;
	}

	MoveComp->MaxWalkSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
}
//end 3_12_Sprint updated for Omega deterministic movement 7_14

//new 3_14 crouch
void ALGSCoreJan12026Character::OnCrouchStarted()
{
	Crouch();

	if (bIsCrouched)
	{
		bSprintHeld = false;
		UpdateSprintState();
	}

	UE_LOG(LogTemplateCharacter, Warning, TEXT("[MOVE] Crouch Start crouched=%d"),
		bIsCrouched ? 1 : 0);
}

void ALGSCoreJan12026Character::OnCrouchReleased()
{
	UnCrouch();

	UE_LOG(LogTemplateCharacter, Warning, TEXT("[MOVE] Crouch Stop crouched=%d"),
		bIsCrouched ? 1 : 0);
}


bool ALGSCoreJan12026Character::IsAirWalkActive() const
{
	return AirWalkComp ? AirWalkComp->IsAirWalkActive() : false;
}


//new 7_14 Omega
void ALGSCoreJan12026Character::OnOmegaDrivePressed()
{
	if (bOmegaDriveActive)
	{
		UE_LOG(
			LogTemplateCharacter,
			Warning,
			TEXT("[OMD] O ignored - Omega already active")
		);
		return;
	}

	if (!IsOmegaDriveReady())
	{
		UE_LOG(
			LogTemplateCharacter,
			Warning,
			TEXT("[OMD] O pressed, but HD + OD alignment is incomplete")
		);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				1.5f,
				FColor::Silver,
				TEXT("[OMD] Requires HyperDrive + OverDrive")
			);
		}

		return;
	}

	ActivateOmegaDrive();
}

void ALGSCoreJan12026Character::ActivateOmegaDrive()
{
	if (bOmegaDriveActive || !IsOmegaDriveReady())
	{
		return;
	}

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();

	if (!MoveComp ||
		!FirstPersonCameraComponent ||
		!OmegaThirdPersonCamera ||
		!GetMesh())
	{
		UE_LOG(
			LogTemplateCharacter,
			Error,
			TEXT("[OMD] Activation blocked - required component missing")
		);
		return;
	}

	bOmegaDriveActive = true;

	// Remember the human configuration.
	// Omega is allowed to distort movement, not forget how to put it back.
	PreOmegaWalkSpeed = MoveComp->MaxWalkSpeed;
	PreOmegaJumpZVelocity = MoveComp->JumpZVelocity;
	bPreOmegaUseControllerRotationYaw = bUseControllerRotationYaw;
	bPreOmegaOrientRotationToMovement =
		MoveComp->bOrientRotationToMovement;

	if (Mesh1P)
	{
		Mesh1P->SetHiddenInGame(true, true);
		Mesh1P->SetVisibility(false, true);
	}

	if (RangedWeaponVisual)
	{
		RangedWeaponVisual->SetHiddenInGame(true, true);
		RangedWeaponVisual->SetVisibility(false, true);
	}

	if (MeleeWeaponVisual)
	{
		MeleeWeaponVisual->SetHiddenInGame(true, true);
		MeleeWeaponVisual->SetVisibility(false, true);
	}

	// Today: Serath.
	// Tomorrow: the holographic body that finally learned how to leave Blender.
	GetMesh()->SetHiddenInGame(false, true);
	GetMesh()->SetVisibility(true, true);

	FirstPersonCameraComponent->SetActive(false);
	OmegaThirdPersonCamera->SetActive(true);

	MoveComp->MaxWalkSpeed = OmegaWalkSpeed;
	MoveComp->JumpZVelocity = OmegaJumpZVelocity;

	bUseControllerRotationYaw = false;
	MoveComp->bOrientRotationToMovement = true;
	MoveComp->RotationRate = FRotator(0.f, 720.f, 0.f);

	GetWorldTimerManager().ClearTimer(Timer_OmegaDriveDuration);
	GetWorldTimerManager().SetTimer(
		Timer_OmegaDriveDuration,
		this,
		&ALGSCoreJan12026Character::DeactivateOmegaDrive,
		OmegaDriveDuration,
		false
	);

	UE_LOG(
		LogTemplateCharacter,
		Warning,
		TEXT("[OMD] ACTIVATED Duration=%.1f Speed=%.1f Jump=%.1f"),
		OmegaDriveDuration,
		OmegaWalkSpeed,
		OmegaJumpZVelocity
	);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.f,
			FColor::Cyan,
			TEXT("[OMD] THIRD-PERSON MANIFESTATION")
		);
	}
}

void ALGSCoreJan12026Character::DeactivateOmegaDrive()
{
	if (!bOmegaDriveActive)
	{
		return;
	}

	bOmegaDriveActive = false;

	GetWorldTimerManager().ClearTimer(Timer_OmegaDriveDuration);

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();

	if (MoveComp)
	{
		MoveComp->MaxWalkSpeed = PreOmegaWalkSpeed;
		MoveComp->JumpZVelocity = PreOmegaJumpZVelocity;
		MoveComp->bOrientRotationToMovement =
			bPreOmegaOrientRotationToMovement;
	}

	bUseControllerRotationYaw = bPreOmegaUseControllerRotationYaw;

	if (OmegaThirdPersonCamera)
	{
		OmegaThirdPersonCamera->SetActive(false);
	}

	if (FirstPersonCameraComponent)
	{
		FirstPersonCameraComponent->SetActive(true);
	}

	if (GetMesh())
	{
		GetMesh()->SetHiddenInGame(true, true);
		GetMesh()->SetVisibility(false, true);
	}

	if (Mesh1P)
	{
		Mesh1P->SetHiddenInGame(false, true);
		Mesh1P->SetVisibility(true, true);
	}

	// Omega may temporarily erase the weapon from sight,
	// but it should not erase the player's last decision.
	if (CombatCore)
	{
		ApplyWeaponVisualsForMode(CombatCore->GetCombatMode());
	}

	UE_LOG(
		LogTemplateCharacter,
		Warning,
		TEXT("[OMD] Finished - returning to first-person reality")
	);
}
//end 7_14


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
	AttachAndApply(MeleeWeaponVisual, MeleeWeaponData, TEXT("Melee"));

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

//new 4_14_AirWalk
void ALGSCoreJan12026Character::OnAirWalkStarted()
{
	if (AirWalkComp)
	{
		AirWalkComp->HandlePress();
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("[AIR] AirWalkComp is NULL"));
	}
}

void ALGSCoreJan12026Character::OnAirWalkReleased()
{
	if (AirWalkComp)
	{
		AirWalkComp->HandleRelease();
	}
}
//end 4_14

//new 4_15 Airwalk Update
void ALGSCoreJan12026Character::CancelSprintForAirWalk()
{
	bSprintHeld = false;
	UpdateSprintState();

	UE_LOG(LogTemplateCharacter, Warning, TEXT("[MOVE] Sprint canceled for AirWalk"));
}
//end 4_15

//HyperDrive 1_27_26:  Careful adding HyperRail 
//new 3_27 AirWalk
//new HyperDrive 1_27_26:  Careful adding HyperRail
void ALGSCoreJan12026Character::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (AirWalkComp)
	{
		AirWalkComp->HandleLanded(Hit);
	}

	if (HyperDriveComp)
	{
		HyperDriveComp->HandleLanded(Hit);
	}
}
//end 1_27_26 & 3_27

//new 5_15 EventHorizon Enablement
void ALGSCoreJan12026Character::OnPrimaryStarted()
{
	if (CombatCore && !CombatCore->IsRangedMode())
	{
		if (OverDriveComp)
		{
			OverDriveComp->StartEventHorizonCharge();
			return;
		}
	}

	if (CombatCore)
	{
		CombatCore->StartAutoFire();
	}
}

//new 5_15 updated So quick click is normal Swing, while hold is EH
void ALGSCoreJan12026Character::OnPrimaryReleased()
{
	if (CombatCore && !CombatCore->IsRangedMode())
	{
		if (OverDriveComp && OverDriveComp->bChargingEventHorizon)
		{
			// Quick click = regular mace swing.
			if (OverDriveComp->GetEventHorizonHeldTime() < OverDriveComp->EventHorizonMinChargeTime)
			{
				OverDriveComp->CancelEventHorizonCharge();
				CombatCore->TryPrimary();
				return;
			}

			// Hold long enough = Event Horizon.
			OverDriveComp->ReleaseEventHorizon();
			return;
		}
	}

	if (CombatCore)
	{
		CombatCore->StopAutoFire();
	}
}
//end 5_15

//new 7_14 OmegaDrive First Pass

bool ALGSCoreJan12026Character::IsOmegaDriveReady() const
{
	if (!HyperDriveComp || !OverDriveComp)
	{
		return false;
	}

	return HyperDriveComp->IsHyperDriveActive()
		&& OverDriveComp->IsOverDriveActive();
}

//
// bool ALGSCoreJan12026Character::IsOmegaDriveReady() const
// {
// 	return HyperDriveComp
// 		&& OverDriveComp
// 		&& HyperDriveComp->IsHyperDriveActive()
// 		&& OverDriveComp->IsOverDriveActive();
// }

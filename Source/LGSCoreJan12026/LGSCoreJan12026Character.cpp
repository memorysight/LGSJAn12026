
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

void ALGSCoreJan12026Character::UpdateSprintState()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		return;
	}

	// For first pass, sprint is simply "button held".
	// Later we can require move input, crouch state, slide state, etc.
	bIsSprinting = bSprintHeld;

	MoveComp->MaxWalkSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
}
//end 3_12_Sprint

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

//end_3_14

//new 3_27 AirWalk
// void ALGSCoreJan12026Character::Tick(float DeltaSeconds)
// {
// 	Super::Tick(DeltaSeconds);
//
// 	UpdateAirWalk(DeltaSeconds);
// }
//
// void ALGSCoreJan12026Character::OnAirWalkStarted()
// {
// 	if (!bHasAirWalkStrand)
// 	{
// 		UE_LOG(LogTemplateCharacter, Warning, TEXT("[AIR] No AirWalk strand"));
// 		return;
// 	}
//
// 	bAirWalkHeld = true;
// 	AirWalkHoldTime = 0.f;
// 	bApexRollConsumed = false;
//
// 	UE_LOG(LogTemplateCharacter, Warning, TEXT("[AIR] RMB started"));
// }
//
// void ALGSCoreJan12026Character::OnAirWalkReleased()
// {
// 	if (!bHasAirWalkStrand)
// 	{
// 		return;
// 	}
//
// 	const bool bWasLifting = bAirLiftActive;
// 	const float HeldTime = AirWalkHoldTime;
//
// 	bAirWalkHeld = false;
// 	AirWalkHoldTime = 0.f;
//
// 	if (bWasLifting)
// 	{
// 		EndAirLift(false);
// 		FallGracefullyWithVelocityChanger();
// 		EvaluateAirWalkApexRNG();
//
// 		UE_LOG(LogTemplateCharacter, Warning, TEXT("[AIR] RMB released -> end lift"));
// 		return;
// 	}
//
// 	if (HeldTime < HoldThreshold)
// 	{
// 		PerformAirWalkTap();
// 		UE_LOG(LogTemplateCharacter, Warning, TEXT("[AIR] RMB tap -> AirWalk"));
// 	}
// }
//
// void ALGSCoreJan12026Character::PerformAirWalkTap()
// {
// 	if (bIsCrouched)
// 	{
// 		UnCrouch();
// 	}
//
// 	bSprintHeld = false;
// 	UpdateSprintState();
//
// 	const bool bIsFallingNow = GetCharacterMovement() && GetCharacterMovement()->IsFalling();
// 	const float UseImpulse = bIsFallingNow ? TapAirWalkImpulseAir : TapAirWalkImpulseGround;
//
// 	LaunchCharacter(FVector(0.f, 0.f, UseImpulse), false, true);
//
// 	AirWalkState = EAirWalkState::TapRise;
// 	bApexRollConsumed = false;
//
// 	UE_LOG(LogTemplateCharacter, Warning, TEXT("[AIR] Tap impulse=%.1f falling=%d"),
// 		UseImpulse, bIsFallingNow ? 1 : 0);
// }
//
// void ALGSCoreJan12026Character::UpdateAirWalk(float DeltaSeconds)
// {
// 	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
// 	if (!MoveComp) return;
//
// 	if (bAirWalkHeld && !bAirLiftActive)
// 	{
// 		AirWalkHoldTime += DeltaSeconds;
//
// 		if (AirWalkHoldTime >= HoldThreshold)
// 		{
// 			BeginAirLift();
// 		}
// 	}
//
// 	if (bAirLiftActive)
// 	{
// 		if (AirWalkEnergyCurrent <= 0.f)
// 		{
// 			EndAirLift(true);
// 			FallGracefullyWithVelocityChanger();
// 			EvaluateAirWalkApexRNG();
// 			return;
// 		}
//
// 		AirWalkEnergyCurrent = FMath::Max(0.f, AirWalkEnergyCurrent - LiftEnergyDrainPerSecond * DeltaSeconds);
//
// 		FVector V = MoveComp->Velocity;
// 		V.Z = FMath::Min(V.Z + (LiftAccelerationZ * DeltaSeconds), LiftMaxUpVelocity);
// 		MoveComp->Velocity = V;
//
// 		// Slightly softer gravity during lift
// 		MoveComp->GravityScale = LiftGravityScale;
// 	}
//
// 	// Apex detection for tap-rise
// 	if (!bAirLiftActive && !bApexRollConsumed && MoveComp->IsFalling())
// 	{
// 		const float AbsZ = FMath::Abs(MoveComp->Velocity.Z);
// 		if (AbsZ <= ApexVelocityThreshold)
// 		{
// 			EvaluateAirWalkApexRNG();
// 		}
// 	}
// }
//
// void ALGSCoreJan12026Character::BeginAirLift()
// {
// 	if (bAirLiftActive) return;
//
// 	if (bIsCrouched)
// 	{
// 		UnCrouch();
// 	}
//
// 	bSprintHeld = false;
// 	UpdateSprintState();
//
// 	bAirLiftActive = true;
// 	AirWalkState = EAirWalkState::Lift;
//
// 	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
// 	{
// 		MoveComp->GravityScale = LiftGravityScale;
// 	}
//
// 	UE_LOG(LogTemplateCharacter, Warning, TEXT("[AIR] BeginAirLift energy=%.1f"), AirWalkEnergyCurrent);
// }
//
// void ALGSCoreJan12026Character::EndAirLift(bool bFromEnergyDepletion)
// {
// 	if (!bAirLiftActive) return;
//
// 	bAirLiftActive = false;
// 	AirWalkState = EAirWalkState::GracefulFall;
//
// 	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
// 	{
// 		MoveComp->GravityScale = GracefulFallGravityScale;
// 	}
//
// 	UE_LOG(LogTemplateCharacter, Warning, TEXT("[AIR] EndAirLift depleted=%d energy=%.1f"),
// 		bFromEnergyDepletion ? 1 : 0,
// 		AirWalkEnergyCurrent);
// }
//
// void ALGSCoreJan12026Character::FallGracefullyWithVelocityChanger()
// {
// 	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
// 	if (!MoveComp) return;
//
// 	FVector V = MoveComp->Velocity;
//
// 	// soften harsh downward snap
// 	if (V.Z < -600.f)
// 	{
// 		V.Z = -600.f;
// 	}
//
// 	MoveComp->Velocity = V;
// 	MoveComp->GravityScale = GracefulFallGravityScale;
//
// 	UE_LOG(LogTemplateCharacter, Warning, TEXT("[AIR] Graceful fall engaged velZ=%.1f"), V.Z);
// }
// void ALGSCoreJan12026Character::EvaluateAirWalkApexRNG()
// {
// 	if (bApexRollConsumed) return;
// 	bApexRollConsumed = true;
//
// 	const float Roll = FMath::FRand();
//
// 	if (Roll <= GodAirBoostChance)
// 	{
// 		bGodAirBoostAvailable = true;
//
// 		LaunchCharacter(FVector(0.f, 0.f, GodAirBoostImpulse), false, true);
//
// 		if (UWorld* World = GetWorld())
// 		{
// 			World->GetTimerManager().ClearTimer(Timer_GodAirBoostReset);
// 			World->GetTimerManager().SetTimer(
// 				Timer_GodAirBoostReset,
// 				this,
// 				&ALGSCoreJan12026Character::ResetGodAirBoost,
// 				0.35f,
// 				false
// 			);
// 		}
//
// 		UE_LOG(LogTemplateCharacter, Warning, TEXT("[AIR] GOD BOOST! roll=%.3f"), Roll);
// 	}
// 	else
// 	{
// 		UE_LOG(LogTemplateCharacter, Warning, TEXT("[AIR] No god boost. roll=%.3f"), Roll);
// 	}
// }
//
// void ALGSCoreJan12026Character::ResetGodAirBoost()
// {
// 	bGodAirBoostAvailable = false;
// }
//
//
// //end 3_27

bool ALGSCoreJan12026Character::IsAirWalkActive() const
{
	return AirWalkComp ? AirWalkComp->IsAirWalkActive() : false;
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

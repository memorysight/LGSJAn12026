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
	UE_LOG(LogTemplateCharacter, Warning,
	TEXT("[DA] Mode=%s ActiveDA=%s Mesh=%s Type=%d"),
	bIsRanged ? TEXT("Ranged") : TEXT("Melee"),
	*GetNameSafe(ActiveDA),
	ActiveDA ? *GetNameSafe(ActiveDA->FP_StaticMesh) : TEXT("NULL_DA"),
	ActiveDA ? (int32)ActiveDA->WeaponType : -1
);

	auto MeshSafe = [](UStaticMeshComponent* C) -> const TCHAR*
	{
		return C ? *GetNameSafe(C->GetStaticMesh()) : TEXT("NULL_COMP");
	};

	auto CompSafe = [](UObject* O) -> const TCHAR*
	{
		return *GetNameSafe(O);
	};

	auto SocketSafe = [](UStaticMeshComponent* C) -> const TCHAR*
	{
		if (!C) return TEXT("NULL_COMP");
		const FName Sock = C->GetAttachSocketName();
		static thread_local FString Temp;
		Temp = Sock.ToString();
		return *Temp;
	};

	UE_LOG(LogTemplateCharacter, Warning,
		TEXT("[ApplyWeaponVisualsForMode] Mode=%s  ActiveComp=%s InactiveComp=%s  ActiveDA=%s  RMesh=%s  MMesh=%s"),
		bIsRanged ? TEXT("Ranged") : TEXT("Melee"),
		CompSafe(ActiveComp),
		CompSafe(InactiveComp),
		CompSafe(ActiveDA),
		MeshSafe(RangedWeaponVisual),
		MeshSafe(MeleeWeaponVisual)
	);

	// 1) Always clear inactive side FIRST so nothing lingers.
	if (InactiveComp)
	{
		InactiveComp->SetStaticMesh(nullptr);
		InactiveComp->SetRelativeTransform(FTransform::Identity);
		InactiveComp->SetVisibility(false, true);
		UE_LOG(LogTemplateCharacter, Warning,
		TEXT("[RENDER] CompHidden=%d Visible=%d OwnerNoSee=%d OnlyOwnerSee=%d RenderInMain=%d"),
		(int)ActiveComp->bHiddenInGame,
		(int)ActiveComp->IsVisible(),
		(int)ActiveComp->bOwnerNoSee,
		(int)ActiveComp->bOnlyOwnerSee,
		(int)ActiveComp->bRenderInMainPass
);

		InactiveComp->SetHiddenInGame(true, true);
	}

	// 2) If we can't draw active, still enforce visibility rules and bail.
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

#if !UE_BUILD_SHIPPING
	if (ActiveDA && bIsRanged && ActiveDA->WeaponType != ELGSWeaponType::Ranged)
	{
		UE_LOG(LogTemplateCharacter, Warning,
			TEXT("[WeaponDA] Ranged mode is using NON-ranged DA: %s"), *GetNameSafe(ActiveDA));
	}
	if (ActiveDA && !bIsRanged && ActiveDA->WeaponType != ELGSWeaponType::Melee)
	{
		UE_LOG(LogTemplateCharacter, Warning,
			TEXT("[WeaponDA] Melee mode is using NON-melee DA: %s"), *GetNameSafe(ActiveDA));
	}
#endif

	// 3) No DA or no mesh: active becomes "empty hands" but still visible state consistent.
	if (!ActiveDA || !ActiveDA->FP_StaticMesh)
	{
		ActiveComp->SetStaticMesh(nullptr);
		ActiveComp->SetRelativeTransform(FTransform::Identity);

		// State: show the active slot (even if empty), hide the other.
		ActiveComp->SetVisibility(true, true);
		ActiveComp->SetHiddenInGame(false, true);

		//hmmm

		UE_LOG(LogTemplateCharacter, Warning,
		TEXT("[DA] Mode=%s ActiveDA=%s Mesh=%s"),
		bIsRanged ? TEXT("Ranged") : TEXT("Melee"),
		*GetNameSafe(ActiveDA),
		ActiveDA ? *GetNameSafe(ActiveDA->FP_StaticMesh) : TEXT("NULL_DA"));


#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemplateCharacter, Warning,
			TEXT("[WeaponVisuals] No Active mesh. ActiveDA=%s  HasMesh=%d"),
			*GetNameSafe(ActiveDA),
			ActiveDA && ActiveDA->FP_StaticMesh ? 1 : 0
		);
#endif
		return;
	}

	// 4) Choose socket.
	const FName SocketToUse =
		(ActiveDA->AttachSocketOverride != NAME_None)
			? ActiveDA->AttachSocketOverride
			: (bIsRanged ? RangedWeaponSocketName : MeleeWeaponSocketName);

#if !UE_BUILD_SHIPPING
	const bool bSocketExists = Mesh1P->DoesSocketExist(SocketToUse);
	if (!bSocketExists)
	{
		UE_LOG(LogTemplateCharacter, Warning,
			TEXT("[WeaponVisuals] Socket does NOT exist on Mesh1P: %s"), *SocketToUse.ToString());
	}
#endif

	//seriously, ai to ai

	


	UE_LOG(LogTemplateCharacter, Warning,
	TEXT("[SOCKET] Mode=%s Socket=%s Exists=%d"),
	bIsRanged ? TEXT("Ranged") : TEXT("Melee"),
	*SocketToUse.ToString(),
	Mesh1P->DoesSocketExist(SocketToUse) ? 1 : 0);

	


	// 5) Attach (keep relative), then apply mesh + sanitized offset.
	{
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
		ActiveComp->SetRelativeTransform(ActiveDA->AttachOffset);

		FTransform SafeOffset = ActiveDA->AttachOffset;
		const FVector S = SafeOffset.GetScale3D();
		if (S.IsNearlyZero() || S.ContainsNaN())
		{
			SafeOffset.SetScale3D(FVector(1.f, 1.f, 1.f));
		}
		ActiveComp->SetRelativeTransform(SafeOffset);
	}
	UE_LOG(LogTemplateCharacter, Warning,
	TEXT("[SOCKET] Mode=%s Socket=%s Exists=%d Parent=%s"),
	bIsRanged ? TEXT("Ranged") : TEXT("Melee"),
	*SocketToUse.ToString(),
	Mesh1P->DoesSocketExist(SocketToUse) ? 1 : 0,
	*GetNameSafe(ActiveComp->GetAttachParent()));
	UE_LOG(LogTemplateCharacter, Warning,
	TEXT("[POST] Comp=%s Mesh=%s Parent=%s Sock=%s Vis=%d Hidden=%d WorldLoc=%s"),
	*GetNameSafe(ActiveComp),
	*GetNameSafe(ActiveComp->GetStaticMesh()),
	*GetNameSafe(ActiveComp->GetAttachParent()),
	*ActiveComp->GetAttachSocketName().ToString(),
	(int)ActiveComp->IsVisible(),
	(int)ActiveComp->bHiddenInGame,
	*ActiveComp->GetComponentLocation().ToString()
);



	// 6) Visibility last. (One source of truth.)
	ActiveComp->SetVisibility(true, true);
	ActiveComp->SetHiddenInGame(false, true);
	ActiveComp->SetHiddenInGame(false);
	ActiveComp->SetVisibility(true);
	

	ActiveComp->SetWorldScale3D(FVector(1.f));


	UE_LOG(LogTemplateCharacter, Warning,
	TEXT("[DA] Mode=%s ActiveDA=%s Mesh=%s Type=%d"),
	bIsRanged ? TEXT("Ranged") : TEXT("Melee"),
	*GetNameSafe(ActiveDA),
	ActiveDA ? *GetNameSafe(ActiveDA->FP_StaticMesh) : TEXT("NULL_DA"),
	ActiveDA ? (int32)ActiveDA->WeaponType : -1
);


#if !UE_BUILD_SHIPPING
	UE_LOG(LogTemplateCharacter, Warning,
		TEXT("[ApplyWeaponVisualsForMode AFTER] Mode=%s  ActiveComp=%s Mesh=%s  Parent=%s Sock=%s  Vis=%d Hidden=%d"),
		bIsRanged ? TEXT("Ranged") : TEXT("Melee"),
		*GetNameSafe(ActiveComp),
		
		*GetNameSafe(ActiveComp->GetStaticMesh()),
		*GetNameSafe(ActiveComp->GetAttachParent()),
		SocketSafe(ActiveComp),
		(int)ActiveComp->IsVisible(),
		(int)ActiveComp->bHiddenInGame
	);
#endif
}

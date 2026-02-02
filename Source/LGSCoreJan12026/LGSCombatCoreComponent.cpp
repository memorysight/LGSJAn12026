#include "LGSCombatCoreComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "LGSCoreJan12026Character.h"


ULGSCombatCoreComponent::ULGSCombatCoreComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//new 1_30 assess switch state mixup
void ULGSCombatCoreComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemplateCharacter, Warning,
		TEXT("[MODE] CombatCore BeginPlay pre-broadcast = %s (Owner=%s, Comp=%s)"),
		CombatMode == ECombatMode::Ranged ? TEXT("Ranged") : TEXT("Melee"),
		GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
		*GetName());

	OnCombatModeChanged.Broadcast(CombatMode);

	UE_LOG(LogTemplateCharacter, Warning,
		TEXT("[MODE] CombatCore BeginPlay post-broadcast = %s"),
		CombatMode == ECombatMode::Ranged ? TEXT("Ranged") : TEXT("Melee"));
}
//end 1_30


//new 1_30 
void ULGSCombatCoreComponent::ToggleCombatMode()
{
	const ECombatMode Current = CombatMode;
	const ECombatMode Next = (CombatMode == ECombatMode::Ranged) ? ECombatMode::Melee : ECombatMode::Ranged;

	auto ToStr = [](ECombatMode M){ return (M == ECombatMode::Ranged) ? TEXT("Ranged") : TEXT("Melee"); };

	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green,
		FString::Printf(TEXT("Toggle: %s -> %s"), ToStr(Current), ToStr(Next)));

	SetCombatMode(Next);
}
//end 1_30


void ULGSCombatCoreComponent::SetCombatMode(ECombatMode NewMode)
{
	UE_LOG(LogTemplateCharacter, Warning, TEXT("[MODE] SetCombatMode %s -> %s"),
		CombatMode == ECombatMode::Ranged ? TEXT("Ranged") : TEXT("Melee"),
		NewMode    == ECombatMode::Ranged ? TEXT("Ranged") : TEXT("Melee"));

	if (CombatMode == NewMode) return;

	CombatMode = NewMode; // ✅ use NewMode
	OnCombatModeChanged.Broadcast(CombatMode);

	UE_LOG(LogTemplateCharacter, Warning, TEXT("[MODE] Now in %s"),
		CombatMode == ECombatMode::Ranged ? TEXT("Ranged") : TEXT("Melee"));
}


void ULGSCombatCoreComponent::TryPrimary()
{
	if (CombatMode == ECombatMode::Ranged)
	{
		DoRangedShot();
	}
	else
	{
		DoMeleeSwing();
	}
}

void ULGSCombatCoreComponent::TrySecondary()
{
	// Reserved for Aim / Block / AltFire later.
}

void ULGSCombatCoreComponent::DoRangedShot()
{
	if (!bCanFire) return;
	bCanFire = false;

	UWorld* World = GetWorld();
	if (!World) { ResetFire(); return; }

	ALGSCoreJan12026Character* OwnerChar = Cast<ALGSCoreJan12026Character>(GetOwner());
	if (!OwnerChar) { ResetFire(); return; }

	if (!BulletClass)
	{
		World->GetTimerManager().SetTimer(Timer_FireCooldown, this, &ULGSCombatCoreComponent::ResetFire, FireCooldown, false);
		return;
	}
	//change to make rider build again to fix desync issue
	//new 1_3_26
	// Prefer muzzle socket location, but aim with camera rotation for FPS feel.
	// Always initialize to something valid.
	FTransform SpawnXform = OwnerChar->GetActorTransform();

	UCameraComponent* Cam = OwnerChar->GetFirstPersonCameraComponent();
	const FRotator AimRot = Cam ? Cam->GetComponentRotation() : OwnerChar->GetActorRotation();

	if (USkeletalMeshComponent* Mesh1P = OwnerChar->GetMesh1P())
	{
		if (Mesh1P->DoesSocketExist(MuzzleSocketName))
		{
			// Use socket location, but aim where the camera looks
			const FVector MuzzleLoc = Mesh1P->GetSocketLocation(MuzzleSocketName);
			SpawnXform = FTransform(AimRot, MuzzleLoc);
		}
		else if (Cam)
		{
			// No socket: spawn a bit in front of camera
			const FVector Loc = Cam->GetComponentLocation() + Cam->GetForwardVector() * 100.f;
			SpawnXform = FTransform(AimRot, Loc);
		}
	}
	else if (Cam)
	{
		// Mesh missing: still allow shooting
		const FVector Loc = Cam->GetComponentLocation() + Cam->GetForwardVector() * 100.f;
		SpawnXform = FTransform(AimRot, Loc);
	}
	//end 1_3_26

	FActorSpawnParameters Params;
	Params.Owner = OwnerChar;
	Params.Instigator = OwnerChar;

	World->SpawnActor<AActor>(BulletClass, SpawnXform, Params);

	World->GetTimerManager().SetTimer(Timer_FireCooldown, this, &ULGSCombatCoreComponent::ResetFire, FireCooldown, false);
}


//1_30 combat core swing for the fences
void ULGSCombatCoreComponent::DoMeleeSwing()
{
	if (!bCanMelee) return;
	bCanMelee = false;

	UWorld* World = GetWorld();
	if (!World) { ResetMelee(); return; }

	ALGSCoreJan12026Character* OwnerChar = Cast<ALGSCoreJan12026Character>(GetOwner());
	if (!OwnerChar) { ResetMelee(); return; }

	if (MeleeMontage)
	{
		if (USkeletalMeshComponent* Arms = OwnerChar->GetMesh1P())
		{
			if (UAnimInstance* AnimInst = Arms->GetAnimInstance())
			{
				AnimInst->Montage_Play(MeleeMontage);
			}
			else
			{
				UE_LOG(LogTemplateCharacter, Warning, TEXT("[MELEE] Mesh1P has no AnimInstance (AnimBP missing?)"));
			}
		}
	}

	World->GetTimerManager().SetTimer(
		Timer_MeleeCooldown, this, &ULGSCombatCoreComponent::ResetMelee, MeleeCooldown, false);
}
//end 1_30

//new 2/2
void ULGSCombatCoreComponent::BeginMeleeDamage()
{
	UE_LOG(LogTemplateCharacter, Warning, TEXT("[MELEE] BeginMeleeDamage called"));

	bMeleeDamageActive = true;
	HitActorsThisSwing.Reset();

	StartMeleeTraceLoop();
}

void ULGSCombatCoreComponent::EndMeleeDamage()
{
	UE_LOG(LogTemplateCharacter, Warning, TEXT("[MELEE] EndMeleeDamage called"));

	bMeleeDamageActive = false;

	StopMeleeTraceLoop();
	HitActorsThisSwing.Reset();
}

void ULGSCombatCoreComponent::StartMeleeTraceLoop()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// ~60 fps trace; you can bump to 0.03f if you want cheaper
	const float TraceInterval = 0.016f;

	World->GetTimerManager().ClearTimer(Timer_MeleeTrace);
	World->GetTimerManager().SetTimer(
		Timer_MeleeTrace, this, &ULGSCombatCoreComponent::PerformMeleeTrace,
		TraceInterval, true
	);
}

void ULGSCombatCoreComponent::StopMeleeTraceLoop()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(Timer_MeleeTrace);
	}
}

void ULGSCombatCoreComponent::PerformMeleeTrace()
{
	if (!bMeleeDamageActive) return;

	UWorld* World = GetWorld();
	if (!World) return;

	ALGSCoreJan12026Character* OwnerChar = Cast<ALGSCoreJan12026Character>(GetOwner());
	if (!OwnerChar) return;

	USkeletalMeshComponent* Arms = OwnerChar->GetMesh1P();
	if (!Arms) return;

	// Start from a socket (WeaponSocket_R by default)
	FVector Start = Arms->GetComponentLocation();
	if (Arms->DoesSocketExist(MeleeTraceSocketName))
	{
		Start = Arms->GetSocketLocation(MeleeTraceSocketName);
	}

	// Aim direction: camera forward for FPS feel
	FVector Dir = OwnerChar->GetActorForwardVector();
	if (UCameraComponent* Cam = OwnerChar->GetFirstPersonCameraComponent())
	{
		Dir = Cam->GetForwardVector();
	}

	const FVector End = Start + (Dir * MeleeTraceDistance);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(MeleeTrace), false);
	Params.AddIgnoredActor(OwnerChar);

	FHitResult Hit;
	const bool bHit = World->SweepSingleByChannel(
		Hit,
		Start,
		End,
		FQuat::Identity,
		MeleeTraceChannel,
		FCollisionShape::MakeSphere(MeleeTraceRadius),
		Params
	);

	if (bDrawMeleeDebug)
	{
		const FColor Color = bHit ? FColor::Red : FColor::Green;
		DrawDebugLine(World, Start, End, Color, false, 0.02f, 0, 1.5f);
		DrawDebugSphere(World, End, MeleeTraceRadius, 12, Color, false, 0.02f);
	}

	if (!bHit) return;

	AActor* HitActor = Hit.GetActor();
	if (!HitActor) return;

	// De-dupe per swing
	if (HitActorsThisSwing.Contains(HitActor))
	{
		return;
	}
	HitActorsThisSwing.Add(HitActor);

	UGameplayStatics::ApplyPointDamage(
		HitActor,
		MeleeDamage,
		Dir,
		Hit,
		OwnerChar->GetController(),
		OwnerChar,
		nullptr // or your custom DamageType later
	);

	UE_LOG(LogTemplateCharacter, Warning, TEXT("[MELEE] Hit %s for %.1f"),
		*GetNameSafe(HitActor), MeleeDamage);
}
//end 2_2

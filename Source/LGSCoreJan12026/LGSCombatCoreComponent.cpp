#include "LGSCombatCoreComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
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


//don't duplicate
void ULGSCombatCoreComponent::TryPrimary()
{
	UE_LOG(LogTemplateCharacter, Warning, TEXT("[INPUT] TryPrimary Mode=%s CanMelee=%d CanFire=%d"),
		CombatMode == ECombatMode::Ranged ? TEXT("Ranged") : TEXT("Melee"),
		bCanMelee, bCanFire);

	if (CombatMode == ECombatMode::Ranged) DoRangedShot();
	else DoMeleeSwing();
}
//_2_1

void ULGSCombatCoreComponent::TrySecondary()
{
	// Reserved for Aim / Block / AltFire later   .
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


//1_30 combat core swing for the fences___TEST PHASE
void ULGSCombatCoreComponent::DoMeleeSwing()
{
	UE_LOG(LogTemplateCharacter, Warning, TEXT("[MELEE] DoMeleeSwing fired (CanMelee=%d Montage=%s)"),
		bCanMelee, *GetNameSafe(MeleeMontage));
	
	if (!bCanMelee) return;
	bCanMelee = false;

	UWorld* World = GetWorld();
	if (!World) { ResetMelee(); return; }

	ALGSCoreJan12026Character* OwnerChar = Cast<ALGSCoreJan12026Character>(GetOwner());
	if (!OwnerChar) { ResetMelee(); return; }

	//possible answer to issue not tested: not built_2_2
	// BeginMeleeDamage();
	// World->GetTimerManager().SetTimerForNextTick([this]()
	// {
	// 	// or use a short timer like 0.15-0.25s to emulate a damage window
	// });
	// World->GetTimerManager().SetTimer(Timer_MeleeTrace, this, &ULGSCombatCoreComponent::EndMeleeDamage, 0.2f, false);


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

//2_1
void ULGSCombatCoreComponent::BeginMeleeDamage()
{
	UE_LOG(LogTemplateCharacter, Warning, TEXT("[MELEE] BeginMeleeDamage"));

	bMeleeDamageActive = true;
	HitActorsThisSwing.Reset();

	StartMeleeTraceLoop();
}

void ULGSCombatCoreComponent::EndMeleeDamage()
{
	UE_LOG(LogTemplateCharacter, Warning, TEXT("[MELEE] EndMeleeDamage"));

	bMeleeDamageActive = false;

	StopMeleeTraceLoop();
	HitActorsThisSwing.Reset();
}

void ULGSCombatCoreComponent::StartMeleeTraceLoop()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(Timer_MeleeTrace);
		World->GetTimerManager().SetTimer(
			Timer_MeleeTrace,
			this,
			&ULGSCombatCoreComponent::PerformMeleeTrace,
			0.016f,   // ~60 FPS
			true
		);
	}
}

void ULGSCombatCoreComponent::StopMeleeTraceLoop()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(Timer_MeleeTrace);
	}
}

//very experimental add
void ULGSCombatCoreComponent::PerformMeleeTrace()
{
	if (!bMeleeDamageActive) return;

	UWorld* World = GetWorld();
	if (!World) return;

	ALGSCoreJan12026Character* OwnerChar = Cast<ALGSCoreJan12026Character>(GetOwner());
	if (!OwnerChar) return;

	USkeletalMeshComponent* Arms = OwnerChar->GetMesh1P();
	if (!Arms) return;

	// --- Start position from socket (or fallback) ---
	const bool bHasSocket = Arms->DoesSocketExist(MeleeTraceSocketName);
	const FVector Start = bHasSocket ? Arms->GetSocketLocation(MeleeTraceSocketName)
									 : Arms->GetComponentLocation();

	// --- Aim direction (camera forward preferred) ---
	FVector Dir = OwnerChar->GetActorForwardVector();
	if (UCameraComponent* Cam = OwnerChar->GetFirstPersonCameraComponent())
	{
		Dir = Cam->GetForwardVector();
	}
	Dir = Dir.GetSafeNormal();

	const FVector End = Start + (Dir * MeleeTraceDistance);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(MeleeTrace), /*bTraceComplex*/ false);
	Params.AddIgnoredActor(OwnerChar);

	// Helpful while debugging:
	// Params.bReturnPhysicalMaterial = true;

	TArray<FHitResult> Hits;

	// SweepMulti so we can see *everything* we touch (capsule/mesh/etc)
	const bool bAnyHit = World->SweepMultiByChannel(
		Hits,
		Start,
		End,
		FQuat::Identity,
		MeleeTraceChannel,
		FCollisionShape::MakeSphere(MeleeTraceRadius),
		Params
	);

	// --- Debug draw ---
	if (bDrawMeleeDebug)
	{
		const FColor Color = bAnyHit ? FColor::Red : FColor::Green;

		DrawDebugLine(World, Start, End, Color, false, 0.03f, 0, 2.0f);
		DrawDebugSphere(World, Start, MeleeTraceRadius, 12, Color, false, 0.03f);
		DrawDebugSphere(World, End,   MeleeTraceRadius, 12, Color, false, 0.03f);

		if (!bHasSocket)
		{
			DrawDebugString(World, Start, TEXT("Socket missing!"), nullptr, FColor::Yellow, 0.03f, false);
		}
	}

	if (!bAnyHit || Hits.Num() == 0)
	{
		// Uncomment for spammy debugging:
		// UE_LOG(LogTemplateCharacter, VeryVerbose, TEXT("[MELEE] No hits"));
		return;
	}

	// Sort by distance so closest hit processes first (optional but nice)
	Hits.Sort([](const FHitResult& A, const FHitResult& B)
	{
		return A.Distance < B.Distance;
	});

	// Optional: one-time per tick log to see what we’re hitting
	UE_LOG(LogTemplateCharacter, Warning,
		TEXT("[MELEE] SweepMulti hit count=%d (Channel=%d)"),
		Hits.Num(), (int32)MeleeTraceChannel);

	for (const FHitResult& H : Hits)
	{
		AActor* HitActor = H.GetActor();
		UPrimitiveComponent* HitComp = H.GetComponent();

		if (!HitActor || HitActor == OwnerChar) continue;

		// Log what we hit (this is the big “Countess mystery” resolver)
		UE_LOG(LogTemplateCharacter, Warning,
			TEXT("    -> %s | Comp=%s | Block=%d | Dist=%.1f"),
			*GetNameSafe(HitActor),
			*GetNameSafe(HitComp),
			H.bBlockingHit ? 1 : 0,
			H.Distance);

		// If you ONLY want to damage characters/pawns, enable this filter:
		// (Turrets likely aren’t pawns; so only enable if desired)
		// if (!HitActor->IsA<APawn>() && !HitActor->IsA<ACharacter>()) continue;

		// Prevent hitting same actor twice in one swing
		if (HitActorsThisSwing.Contains(HitActor)) continue;
		HitActorsThisSwing.Add(HitActor);

		// IMPORTANT:
		// Some collision setups produce non-blocking overlaps.
		// SweepMulti returns both; ApplyDamage works either way.
		const FVector UseDir = Dir;

		// Option A: PointDamage (keeps your existing approach)
		UGameplayStatics::ApplyPointDamage(
			HitActor,
			MeleeDamage,
			UseDir,
			H,
			OwnerChar->GetController(),
			OwnerChar,
			nullptr
		);

		// Option B: Plain ApplyDamage (uncomment to test if Countess only listens to AnyDamage)
		/*
		UGameplayStatics::ApplyDamage(
			HitActor,
			MeleeDamage,
			OwnerChar->GetController(),
			OwnerChar,
			nullptr
		);
		*/

		UE_LOG(LogTemplateCharacter, Warning,
			TEXT("[MELEE] Applied %.1f damage to %s"),
			MeleeDamage, *GetNameSafe(HitActor));

		// If you only want ONE target per swing tick, break here:
		// break;
	}
}
//testing

//end 2_2

void ULGSCombatCoreComponent::ResetFire()
{
	bCanFire = true;
}

void ULGSCombatCoreComponent::ResetMelee()
{
	bCanMelee = true;
}

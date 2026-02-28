#include "LGSCombatCoreComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
//new 2_12
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
//end 2_12
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
	// Reserved for Aim / Block / AltFire later .
}

void ULGSCombatCoreComponent::DoRangedShot()
{
	if (!bCanFire) return;
	bCanFire = false;

	UWorld* World = GetWorld();
	if (!World) { ResetFire(); return; }

	ALGSCoreJan12026Character* OwnerChar = Cast<ALGSCoreJan12026Character>(GetOwner());
	if (!OwnerChar) { ResetFire(); return; }

	//new 2_12 rifleAnimation
	// ✅ Play rifle fire montage (visual)
	// --- Ranged fire montage (first-person arms) ---
	if (FP_Rifle_Shoot_Montage)
	{
		if (USkeletalMeshComponent* Arms = OwnerChar->GetMesh1P())
		{
			if (UAnimInstance* AnimInst = Arms->GetAnimInstance())
			{
				// Optional: don't restart if already playing
				if (!AnimInst->Montage_IsPlaying(FP_Rifle_Shoot_Montage))
				{
					AnimInst->Montage_Play(FP_Rifle_Shoot_Montage, 1.0f);
				}
			}
			else
			{
				UE_LOG(LogTemplateCharacter, Warning, TEXT("[RANGED] Mesh1P has no AnimInstance"));
			}
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("[RANGED] FP_Rifle_Shoot_Montage is NULL"));
	}

	//end 2_12

	if (!BulletClass)
	{
		World->GetTimerManager().SetTimer(Timer_FireCooldown, this, &ULGSCombatCoreComponent::ResetFire, FireCooldown, false);
		return;
	}
	
	//1_3_26
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

//new 2_18 autofire
void ULGSCombatCoreComponent::StartAutoFire()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UE_LOG(LogTemplateCharacter, Warning, TEXT("[AUTO] StartAutoFire FullAuto=%d Mode=%s bIsAutoFiring=%d"),
		bIsFullAuto ? 1 : 0,
		CombatMode == ECombatMode::Ranged ? TEXT("Ranged") : TEXT("Melee"),
		bIsAutoFiring ? 1 : 0);

	if (!bIsFullAuto)
	{
		TryPrimary();
		return;
	}

	if (bIsAutoFiring) return;
	bIsAutoFiring = true;

	TryPrimary();

	const float Interval = FMath::Max(0.01f, FireCooldown);

	World->GetTimerManager().SetTimer(
		Timer_AutoFire,
		this,
		&ULGSCombatCoreComponent::AutoFireTick,
		Interval,
		true
	);

	const bool bActive = World->GetTimerManager().IsTimerActive(Timer_AutoFire);
	UE_LOG(LogTemplateCharacter, Warning, TEXT("[AUTO] Timer_AutoFire active=%d Interval=%.3f"), bActive ? 1 : 0, Interval);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green,
			FString::Printf(TEXT("AUTO START active=%d int=%.2f"), bActive ? 1 : 0, Interval));
	}
}

void ULGSCombatCoreComponent::StopAutoFire()
{
	UE_LOG(LogTemplateCharacter, Warning, TEXT("[AUTO] StopAutoFire"));

	bIsAutoFiring = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(Timer_AutoFire);

		const bool bActive = World->GetTimerManager().IsTimerActive(Timer_AutoFire);
		UE_LOG(LogTemplateCharacter, Warning, TEXT("[AUTO] Timer_AutoFire active(after clear)=%d"), bActive ? 1 : 0);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, TEXT("AUTO STOP"));
		}
	}
}

void ULGSCombatCoreComponent::AutoFireTick()
{
	if (!bIsAutoFiring)
	{
		StopAutoFire();
		return;
	}

	UE_LOG(LogTemplateCharacter, Warning, TEXT("[AUTO] Tick bCanFire=%d"), bCanFire ? 1 : 0);

	TryPrimary();
}


void ULGSCombatCoreComponent::ResetFire()
{
	bCanFire = true;
	UE_LOG(LogTemplateCharacter, Warning, TEXT("[AUTO] ResetFire -> bCanFire=1"));
}
//end 2_18



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
//new_2_29
void ULGSCombatCoreComponent::PerformMeleeTrace()
{
    if (!bMeleeDamageActive) return;

    UWorld* World = GetWorld();
    if (!World) return;

    ALGSCoreJan12026Character* OwnerChar = Cast<ALGSCoreJan12026Character>(GetOwner());
    if (!OwnerChar) return;

    UStaticMeshComponent* Weapon = OwnerChar->GetMeleeWeaponVisual();
    if (!Weapon || !Weapon->GetStaticMesh())
    {
        UE_LOG(LogTemplateCharacter, Warning, TEXT("[MELEE] No MeleeWeaponVisual or StaticMesh"));
        return;
    }

    FCollisionQueryParams Params(SCENE_QUERY_STAT(MeleeTrace), false);
    Params.AddIgnoredActor(OwnerChar);

    auto DoWeaponSweep = [&](FName StartSocket, FName EndSocket)
    {
        if (!Weapon->DoesSocketExist(StartSocket) || !Weapon->DoesSocketExist(EndSocket))
        {
            UE_LOG(LogTemplateCharacter, Warning, TEXT("[MELEE] Missing sockets: %s or %s"),
                *StartSocket.ToString(), *EndSocket.ToString());
            return;
        }

        const FVector Start = Weapon->GetSocketLocation(StartSocket);
        const FVector End   = Weapon->GetSocketLocation(EndSocket);

        TArray<FHitResult> Hits;

        const bool bAnyHit = World->SweepMultiByChannel(
            Hits,
            Start,
            End,
            FQuat::Identity,
            MeleeTraceChannel,
            FCollisionShape::MakeSphere(MeleeTraceRadius),
            Params
        );

        if (bDrawMeleeDebug)
        {
            const FColor Color = bAnyHit ? FColor::Red : FColor::Green;
            DrawDebugLine(World, Start, End, Color, false, 0.03f, 0, 2.0f);
            DrawDebugSphere(World, Start, MeleeTraceRadius, 12, Color, false, 0.03f);
            DrawDebugSphere(World, End,   MeleeTraceRadius, 12, Color, false, 0.03f);
        }

        if (!bAnyHit || Hits.Num() == 0)
        {
            return;
        }

        Hits.Sort([](const FHitResult& A, const FHitResult& B){ return A.Distance < B.Distance; });

        const FVector UseDir = (End - Start).GetSafeNormal();

        for (const FHitResult& H : Hits)
        {
            AActor* HitActor = H.GetActor();
            if (!HitActor || HitActor == OwnerChar) continue;

            if (HitActorsThisSwing.Contains(HitActor)) continue;
            HitActorsThisSwing.Add(HitActor);

            UGameplayStatics::ApplyPointDamage(
                HitActor,
                MeleeDamage,
                UseDir,
                H,
                OwnerChar->GetController(),
                OwnerChar,
                nullptr
            );

            UE_LOG(LogTemplateCharacter, Warning, TEXT("[MELEE] Hit %s via %s->%s"),
                *GetNameSafe(HitActor), *StartSocket.ToString(), *EndSocket.ToString());

            // If you do knockback, do it HERE so it also only happens once per actor per swing.
        }
    };

    // Call sweeps for each cutting edge
    DoWeaponSweep(TEXT("Trace_L_Base"), TEXT("Trace_L_Tip"));
    DoWeaponSweep(TEXT("Trace_R_Base"), TEXT("Trace_R_Tip"));
}
//testing
//end 2_2
//end2_28


void ULGSCombatCoreComponent::ResetMelee()
{
	bCanMelee = true;
}

#include "LGSShieldComponent.h"

#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "LGSCoreJan12026Character.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/DamageType.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"

#include "GameFramework/DamageType.h" 
#include "CollisionQueryParams.h"

#include "CollisionQueryParams.h"

#include "UObject/ConstructorHelpers.h"

#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

ULGSShieldComponent::ULGSShieldComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Defaults
	MaxShieldEnergy = 100.f;
	ShieldEnergy = MaxShieldEnergy;
	ShieldDrainPerSecond = 20.f;
	ShieldRegenPerSecond = 10.f;
	ShieldRegenDelay = 1.5f;

	bShieldActive = false;
	bShieldBroken = false;
	bDestroyBulletsOnOverlap = true;

	// Burst defaults
	bShieldBurstEnabled = true;
	ShieldBurstMinChargePercent = 0.7f;
	ShieldBurstRadius = 600.f;
	ShieldBurstBaseDamage = 50.f;
	ShieldBurstGodDamageMultiplier = 2.0f;
	ShieldBurstDudDamageMultiplier = 0.1f;
	ShieldBurstKnockbackStrength = 2000.f;
	bShieldBurstEligible = true;

	// Setup defaults
	CollisionSphereRadius = 65.f;
}

void ULGSShieldComponent::BeginPlay()
{
	Super::BeginPlay();

	ALGSCoreJan12026Character* OwnerChar = Cast<ALGSCoreJan12026Character>(GetOwner());
	if (!OwnerChar) return;

	ShieldRootComp      = OwnerChar->GetShieldRootComp();
	ShieldCollisionComp = OwnerChar->GetShieldCollisionComp();
	ShieldVisualComp    = OwnerChar->GetShieldVisualComp();

	UE_LOG(LogTemp, Warning, TEXT("[ShieldComp] Root=%s Coll=%s Vis=%s"),
	*GetNameSafe(ShieldRootComp),
	*GetNameSafe(ShieldCollisionComp),
	*GetNameSafe(ShieldVisualComp));


	if (ShieldCollisionComp)
	{
		ShieldCollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ULGSShieldComponent::OnShieldBeginOverlap);
	}

	RefreshShieldVisualState();
}




void ULGSShieldComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopShieldDrain();
	StopShieldRegen();

	// End timeshift if we’re mid-slowmo
	EndBurstTimeShift();

	Super::EndPlay(EndPlayReason);
}

ACharacter* ULGSShieldComponent::GetOwningCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}


void ULGSShieldComponent::ToggleShield()
{
	if (!bShieldActive) ActivateShield();
	else DeactivateShield();
}

void ULGSShieldComponent::ActivateShield()
{
	if (bShieldBroken || ShieldEnergy <= 0.f)
	{
		return;
	}

	bShieldActive = true;
	RefreshShieldVisualState();

	StartShieldDrain();
	StopShieldRegen();
}

void ULGSShieldComponent::DeactivateShield()
{
	if (!bShieldActive)
	{
		return;
	}

	bShieldActive = false;
	RefreshShieldVisualState();

	StopShieldDrain();
	StartShieldRegenWithDelay();
}

float ULGSShieldComponent::HandleIncomingDamage(float DamageAmount, AController* /*EventInstigator*/, AActor* /*DamageCauser*/)
{
	if (IsShieldActiveAndPowered())
	{
		const float Cost = FMath::Max(1.f, DamageAmount);
		ShieldEnergy = FMath::Max(0.f, ShieldEnergy - Cost);

		if (ShieldEnergy <= 0.f)
		{
			BreakShield();
		}

		return 0.f; // fully absorbed
	}

	return DamageAmount; // pass through
}

void ULGSShieldComponent::GetShieldState(float& OutEnergy, float& OutMax, bool& OutActive, bool& OutBroken) const
{
	OutEnergy = ShieldEnergy;
	OutMax = MaxShieldEnergy;
	OutActive = bShieldActive;
	OutBroken = bShieldBroken;
}

void ULGSShieldComponent::ApplyShieldState(float InEnergy, float InMax, bool bInActive, bool bInBroken)
{
	MaxShieldEnergy = InMax;
	ShieldEnergy = FMath::Clamp(InEnergy, 0.f, MaxShieldEnergy);

	bShieldBroken = bInBroken || ShieldEnergy <= 0.f;

	StopShieldDrain();
	StopShieldRegen();

	UpdateShieldBurstEligibility();

	const bool bDesiredActive = bInActive && !bShieldBroken && ShieldEnergy > 0.f;
	if (bDesiredActive) ActivateShield();
	else
	{
		bShieldActive = false;
		RefreshShieldVisualState();
		StartShieldRegenWithDelay();
	}
}

void ULGSShieldComponent::OnShieldBeginOverlap(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/, bool /*bFromSweep*/,
	const FHitResult& /*SweepResult*/)
{
	if (!OtherActor || !bShieldActive)
	{
		return;
	}

	if (!bDestroyBulletsOnOverlap)
	{
		return;
	}

	bool bLooksLikeBullet = false;

	if (BulletClass)
	{
		bLooksLikeBullet = OtherActor->IsA(BulletClass);
	}
	else
	{
		bLooksLikeBullet = OtherActor->GetName().Contains(TEXT("Bullet"));
	}

	if (bLooksLikeBullet)
	{
		OtherActor->Destroy();

		ShieldEnergy = FMath::Max(0.f, ShieldEnergy - 1.f);
		if (ShieldEnergy <= 0.f)
		{
			BreakShield();
		}
	}
}

void ULGSShieldComponent::UpdateShieldBurstEligibility()
{
	if (!bShieldBurstEnabled) return;

	const float Threshold = MaxShieldEnergy * ShieldBurstMinChargePercent;
	if (ShieldEnergy >= Threshold)
	{
		bShieldBurstEligible = true;
	}
}

/* ---------- Drain / Regen ---------- */

void ULGSShieldComponent::StartShieldDrain()
{
	if (!GetWorld()) return;

	if (!GetWorld()->GetTimerManager().IsTimerActive(Timer_ShieldDrain))
	{
		GetWorld()->GetTimerManager().SetTimer(Timer_ShieldDrain, this, &ULGSShieldComponent::DrainTick, 0.1f, true);
	}
}

void ULGSShieldComponent::StopShieldDrain()
{
	if (!GetWorld()) return;
	GetWorld()->GetTimerManager().ClearTimer(Timer_ShieldDrain);
}

void ULGSShieldComponent::DrainTick()
{
	if (!bShieldActive) return;

	const float Step = ShieldDrainPerSecond * 0.1f;
	ShieldEnergy = FMath::Max(0.f, ShieldEnergy - Step);

	if (ShieldEnergy <= 0.f)
	{
		BreakShield();
	}
}

void ULGSShieldComponent::StartShieldRegenWithDelay()
{
	if (!GetWorld()) return;

	StopShieldRegen();

	GetWorld()->GetTimerManager().SetTimer(
		Timer_ShieldRegenDelay,
		FTimerDelegate::CreateUObject(this, &ULGSShieldComponent::StartShieldRegen),
		ShieldRegenDelay,
		false
	);
}

void ULGSShieldComponent::StartShieldRegen()
{
	if (!GetWorld()) return;
	if (bShieldActive) return;

	GetWorld()->GetTimerManager().SetTimer(Timer_ShieldRegen, this, &ULGSShieldComponent::RegenTick, 0.1f, true);
}

void ULGSShieldComponent::StopShieldRegen()
{
	if (!GetWorld()) return;
	GetWorld()->GetTimerManager().ClearTimer(Timer_ShieldRegen);
	GetWorld()->GetTimerManager().ClearTimer(Timer_ShieldRegenDelay);
}

void ULGSShieldComponent::RegenTick()
{
	if (bShieldActive)
	{
		StopShieldRegen();
		return;
	}

	const float Step = ShieldRegenPerSecond * 0.1f;
	ShieldEnergy = FMath::Min(MaxShieldEnergy, ShieldEnergy + Step);

	if (bShieldBroken && ShieldEnergy >= FMath::Max(5.f, 0.05f * MaxShieldEnergy))
	{
		bShieldBroken = false;
	}

	UpdateShieldBurstEligibility();

	if (ShieldEnergy >= MaxShieldEnergy)
	{
		StopShieldRegen();
	}
}

void ULGSShieldComponent::BreakShield()
{
	TriggerShieldBurst();

	ShieldEnergy = 0.f;
	bShieldBroken = true;
	bShieldActive = false;

	RefreshShieldVisualState();

	StopShieldDrain();
	StartShieldRegenWithDelay();
}

/* ---------- Visuals / Collision ---------- */

void ULGSShieldComponent::RefreshShieldVisualState()
{
	const bool bShow = bShieldActive && !bShieldBroken && ShieldEnergy > 0.f;

	if (ShieldVisualComp)
	{
		ShieldVisualComp->SetHiddenInGame(!bShow, true);
	}

	if (ShieldCollisionComp)
	{
		if (bShow)
		{
			ShieldCollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			ShieldCollisionComp->SetGenerateOverlapEvents(true);
		}
		else
		{
			ShieldCollisionComp->SetGenerateOverlapEvents(false);
			ShieldCollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}


/* ---------- Burst / TimeShift ---------- */

void ULGSShieldComponent::TriggerShieldBurst()
{
	if (!bShieldBurstEnabled || !bShieldBurstEligible)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	ACharacter* OwnerChar = GetOwningCharacter();
	if (!OwnerChar) return;

	bShieldBurstEligible = false;

	const FVector Origin = OwnerChar->GetActorLocation();
	const FRotator Rot = OwnerChar->GetActorRotation();

	const float Roll = FMath::FRand();

	float BurstDamage = ShieldBurstBaseDamage;
	bool bApplyKnockback = true;
	UNiagaraSystem* BurstFX = ShieldBurstAverageFX;

	bool bWasGodBurst = false;

	if (Roll < 0.2f)
	{
		BurstDamage = ShieldBurstBaseDamage * ShieldBurstGodDamageMultiplier;
		BurstFX = ShieldBurstGodFX ? ShieldBurstGodFX : ShieldBurstAverageFX;
		bWasGodBurst = true;
	}
	else if (Roll < 0.8f)
	{
		BurstDamage = ShieldBurstBaseDamage;
		BurstFX = ShieldBurstAverageFX;
	}
	else
	{
		BurstDamage = ShieldBurstBaseDamage * ShieldBurstDudDamageMultiplier;
		bApplyKnockback = false;
		BurstFX = ShieldBurstDudFX ? ShieldBurstDudFX : ShieldBurstAverageFX;
	}

	if (BurstFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, BurstFX, Origin, Rot);
	}

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(OwnerChar);



	UGameplayStatics::ApplyRadialDamage(
		this, // WorldContextObject (safe)
		BurstDamage,
		Origin,
		ShieldBurstRadius,
		UDamageType::StaticClass(),
		IgnoreActors,
		OwnerChar,                 // DamageCauser
		OwnerChar->GetController(), // Instigator
		true
	);


	int32 NumEnemiesAffected = 0;

	if (bApplyKnockback && ShieldBurstKnockbackStrength > 0.f)
	{
		TArray<FOverlapResult> OverlapResults;
		FCollisionShape Sphere = FCollisionShape::MakeSphere(ShieldBurstRadius);
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ShieldBurst), false, OwnerChar);

		if (World->OverlapMultiByChannel(
			OverlapResults,
			Origin,
			FQuat::Identity,
			ECC_Pawn,
			Sphere,
			QueryParams))
		{
			for (const FOverlapResult& Result : OverlapResults)
			{
				AActor* Actor = Result.GetActor();
				if (!Actor || Actor == OwnerChar) continue;

				ACharacter* OtherChar = Cast<ACharacter>(Actor);
				if (!OtherChar) continue;

				NumEnemiesAffected++;

				const FVector Dir = (OtherChar->GetActorLocation() - Origin).GetSafeNormal();
				OtherChar->LaunchCharacter(Dir * ShieldBurstKnockbackStrength, true, true);
			}
		}
	}

	MaybeStartBurstTimeShift(NumEnemiesAffected, bWasGodBurst);
}

void ULGSShieldComponent::MaybeStartBurstTimeShift(int32 NumEnemiesAffected, bool bWasGodBurst)
{
	LastBurstEnemiesInRadius = NumEnemiesAffected;

	if (!bBurstTimeShiftEnabled) return;

	if (!bWasGodBurst || NumEnemiesAffected < BurstTimeShiftMinEnemies)
	{
		LastBurstEnemyProximityScore = 0.f;
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	LastBurstEnemyProximityScore = FMath::Clamp(static_cast<float>(NumEnemiesAffected) / 5.f, 0.f, 1.f);

	UGameplayStatics::SetGlobalTimeDilation(World, BurstTimeShiftGlobalDilation);

	// Player "fast in slow-mo": set CustomTimeDilation on the owning character
	if (ACharacter* OwnerChar = GetOwningCharacter())
	{
		OwnerChar->CustomTimeDilation = BurstTimeShiftPlayerDilation;
	}

	World->GetTimerManager().ClearTimer(Timer_BurstTimeShift);
	World->GetTimerManager().SetTimer(
		Timer_BurstTimeShift,
		this,
		&ULGSShieldComponent::EndBurstTimeShift,
		BurstTimeShiftDuration,
		false
	);
}

void ULGSShieldComponent::EndBurstTimeShift()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UGameplayStatics::SetGlobalTimeDilation(World, 1.f);

	if (ACharacter* OwnerChar = GetOwningCharacter())
	{
		OwnerChar->CustomTimeDilation = 1.f;
	}

	World->GetTimerManager().ClearTimer(Timer_BurstTimeShift);
}

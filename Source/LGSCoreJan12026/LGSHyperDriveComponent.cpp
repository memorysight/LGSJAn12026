#include "LGSHyperDriveComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "CollisionShape.h"
#include "CollisionQueryParams.h"
#include "Engine/EngineTypes.h"     // FOverlapResult, etc.
#include "Components/PrimitiveComponent.h"
#include "Engine/OverlapResult.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

ULGSHyperDriveComponent::ULGSHyperDriveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULGSHyperDriveComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache base jump for restore (MEGA BeginPlay behavior)
	if (UCharacterMovementComponent* MoveComp = GetMoveComp())
	{
		BaseJumpZVelocity = MoveComp->JumpZVelocity;
	}
}

ACharacter* ULGSHyperDriveComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}

UCharacterMovementComponent* ULGSHyperDriveComponent::GetMoveComp() const
{
	if (ACharacter* C = GetOwnerCharacter())
	{
		return C->GetCharacterMovement();
	}
	return nullptr;
}

void ULGSHyperDriveComponent::RegisterKill()
{
	if (!GetWorld()) return;

	if (bHyperDriveActive) return;

	KillStreakCount++;

	// Reset streak window timer
	GetWorld()->GetTimerManager().ClearTimer(Timer_KillStreakWindow);
	GetWorld()->GetTimerManager().SetTimer(
		Timer_KillStreakWindow,
		this,
		&ULGSHyperDriveComponent::ResetKillStreak,
		KillStreakWindow,
		false
	);

	if (KillStreakCount >= KillsForHyperDrive)
	{
		BeginHyperDrive();
	}
}

void ULGSHyperDriveComponent::BeginHyperDrive()
{
	if (!GetWorld()) return;
	if (bHyperDriveActive) return;

	bHyperDriveActive = true;
	bHasUsedLandingBurstThisHyperDrive = false;

	// Jump boost
	if (bHyperDriveJumpBoostEnabled)
	{
		if (UCharacterMovementComponent* MoveComp = GetMoveComp())
		{
			if (BaseJumpZVelocity <= 0.f)
			{
				BaseJumpZVelocity = MoveComp->JumpZVelocity;
			}
			MoveComp->JumpZVelocity = BaseJumpZVelocity + HyperDriveJumpZBoost;
		}
	}

	// Stop streak window
	GetWorld()->GetTimerManager().ClearTimer(Timer_KillStreakWindow);

	// Start HyperDrive duration timer
	GetWorld()->GetTimerManager().ClearTimer(Timer_HyperDriveDuration);
	GetWorld()->GetTimerManager().SetTimer(
		Timer_HyperDriveDuration,
		this,
		&ULGSHyperDriveComponent::EndHyperDrive,
		HyperDriveDuration,
		false
	);

	OnHyperDriveStarted.Broadcast();
}

void ULGSHyperDriveComponent::EndHyperDrive()
{
	if (!GetWorld()) return;
	if (!bHyperDriveActive) return;

	bHyperDriveActive = false;
	bHasUsedLandingBurstThisHyperDrive = false;

	// Restore jump
	if (UCharacterMovementComponent* MoveComp = GetMoveComp())
	{
		if (BaseJumpZVelocity > 0.f)
		{
			MoveComp->JumpZVelocity = BaseJumpZVelocity;
		}
	}

	KillStreakCount = 0;
	GetWorld()->GetTimerManager().ClearTimer(Timer_HyperDriveDuration);

	OnHyperDriveEnded.Broadcast();
}

void ULGSHyperDriveComponent::NotifyHyperRailgunHit(int32 EnemiesHit, bool bWasKill)
{
	// 1) bail if disabled
	if (!bHyperRailgunTimeShiftEnabled)
	{
		return;
	}

	// 2) bail if not in HyperDrive
	if (!bHyperDriveActive)
	{
		return;
	}

	// 3) bail if not kill (optional, matches MEGA "finisher" feel)
	if (!bWasKill)
	{
		return;
	}

	// 4) bail if EnemiesHit < min
	if (EnemiesHit < HyperRailgunMinEnemies)
	{
		return;
	}

	// 5) RNG roll vs chance
	const float Roll = FMath::FRand(); // [0,1)
	if (Roll > HyperRailgunTimeShiftChance)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ACharacter* OwnerChar = GetOwnerCharacter();
	if (!OwnerChar)
	{
		return;
	}

	// 6) Apply global dilation (world-impacting)
	UGameplayStatics::SetGlobalTimeDilation(World, HyperRailgunGlobalDilation);

	// 7) Apply player dilation (component-owned)
	OwnerChar->CustomTimeDilation = HyperRailgunPlayerDilation;

	// 8) timer → EndBurstTimeShift
	World->GetTimerManager().ClearTimer(Timer_BurstTimeShift);
	World->GetTimerManager().SetTimer(
		Timer_BurstTimeShift,
		this,
		&ULGSHyperDriveComponent::EndBurstTimeShift,
		HyperRailgunDuration,
		false
	);
}

void ULGSHyperDriveComponent::EndBurstTimeShift()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ACharacter* OwnerChar = GetOwnerCharacter();

	// 9) reset dilations to normal
	UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);

	if (OwnerChar)
	{
		OwnerChar->CustomTimeDilation = 1.0f;
	}

	World->GetTimerManager().ClearTimer(Timer_BurstTimeShift);
}

void ULGSHyperDriveComponent::ResetKillStreak()
{
	if (bHyperDriveActive) return;
	KillStreakCount = 0;
}



// ===== HyperRailgun TimeShift (MEGA 11/28) =====

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperDrive|Railgun|TimeShift")
bool bHyperRailgunTimeShiftEnabled = true;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperDrive|Railgun|TimeShift", meta=(ClampMin="0.0", ClampMax="1.0"))
float HyperRailgunTimeShiftChance = 0.3f; // 30%

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperDrive|Railgun|TimeShift")
float HyperRailgunGlobalDilation = 0.15f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperDrive|Railgun|TimeShift")
float HyperRailgunPlayerDilation = 1.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperDrive|Railgun|TimeShift")
float HyperRailgunDuration = 0.25f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperDrive|Railgun|TimeShift")
int32 HyperRailgunMinEnemies = 1;

UFUNCTION(BlueprintCallable, Category="HyperDrive|Railgun")
void NotifyHyperRailgunHit(int32 EnemiesHit, bool bWasKill);




void ULGSHyperDriveComponent::HandleLanded(const FHitResult& Hit)
{
	// Mirror MEGA: landing burst only if enabled + active + unused
	if (!bLandingBurstEnabled) return;
	if (!bHyperDriveActive) return;
	if (bHasUsedLandingBurstThisHyperDrive) return;

	UCharacterMovementComponent* MoveComp = GetMoveComp();
	if (!MoveComp) return;

	// NOTE: Landed() often has Velocity.Z ~ 0, but keep MEGA logic for MVP.
	const float ImpactSpeed = -MoveComp->Velocity.Z;
	if (ImpactSpeed < LandingBurstMinVelocity) return;

	TriggerLandingBurst();
	bHasUsedLandingBurstThisHyperDrive = true;
}



void ULGSHyperDriveComponent::TriggerLandingBurst()
{
	UWorld* World = GetWorld();
	if (!World) return;

	ACharacter* OwnerChar = GetOwnerCharacter();
	if (!OwnerChar) return;

	const FVector Origin = OwnerChar->GetActorLocation();
	const FRotator Rot   = OwnerChar->GetActorRotation();

	const float Roll = FMath::FRand();

	float BurstDamage = LandingBurstBaseDamage;
	bool  bApplyKnockback = true;
	UNiagaraSystem* BurstFX = LandingBurstAverageFX;

	if (Roll < 0.2f)
	{
		BurstDamage = LandingBurstBaseDamage * LandingBurstGodDamageMultiplier;
		BurstFX = LandingBurstGodFX ? LandingBurstGodFX : LandingBurstAverageFX;
	}
	else if (Roll < 0.8f)
	{
		BurstDamage = LandingBurstBaseDamage;
		BurstFX = LandingBurstAverageFX;
	}
	else
	{
		BurstDamage = LandingBurstBaseDamage * LandingBurstDudDamageMultiplier;
		bApplyKnockback = false;
		BurstFX = LandingBurstDudFX ? LandingBurstDudFX : LandingBurstAverageFX;
	}

	if (BurstFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, BurstFX, Origin, Rot);
	}

	TArray<AActor*> Ignore;
	Ignore.Add(OwnerChar);

	AController* InstigatorController = OwnerChar->GetController();

	UGameplayStatics::ApplyRadialDamage(
		OwnerChar,
		BurstDamage,
		Origin,
		LandingBurstRadius,
		nullptr,
		Ignore,
		OwnerChar,
		InstigatorController,
		true
	);

	if (bApplyKnockback && LandingBurstKnockbackStrength > 0.f)
	{
		TArray<FOverlapResult> Overlaps;
		FCollisionShape Sphere = FCollisionShape::MakeSphere(LandingBurstRadius);
		FCollisionQueryParams Params(SCENE_QUERY_STAT(LandingBurst), false, OwnerChar);

		if (World->OverlapMultiByChannel(Overlaps, Origin, FQuat::Identity, ECC_Pawn, Sphere, Params))
		{
			for (const FOverlapResult& Overlap : Overlaps)
			{
				AActor* OtherActor = Overlap.Component.IsValid() ? Overlap.Component->GetOwner() : nullptr;
				ACharacter* Other = Cast<ACharacter>(OtherActor);

				if (!Other || Other == OwnerChar) continue;

				const FVector Dir = (Other->GetActorLocation() - Origin).GetSafeNormal();
				Other->LaunchCharacter(Dir * LandingBurstKnockbackStrength, true, true);
			}
		}
	}
	
}


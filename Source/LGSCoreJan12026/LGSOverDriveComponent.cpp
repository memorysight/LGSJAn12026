#include "LGSOverDriveComponent.h"

#include "GameFramework/Character.h"
//new 5_13 ODKillStreak
#include "TimerManager.h"
#include "Engine/World.h"
//end 5_13
//new 5_15 EventHorizon
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
//end 5_15
//new 5_15 needed to resolve Hit.GetResult Error
#include "CollisionQueryParams.h"
#include "Engine/OverlapResult.h"
//end 5_15 resolved error!
#include "Engine/Engine.h"

ULGSOverDriveComponent::ULGSOverDriveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULGSOverDriveComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());

	UE_LOG(LogTemp, Warning, TEXT("[OD] OverDrive Component BeginPlay Owner=%s"),
		*GetNameSafe(OwnerCharacter));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.5f,
			FColor::Orange,
			FString::Printf(TEXT("[OD] OverDrive Online: %s"), *GetNameSafe(OwnerCharacter))
		);
	}
}

//new 5_15
// const FVector Direction = ToTarget.GetSafeNormal();
// const float Dot = FVector::DotProduct(Forward, Direction);
// const float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(Dot));
//
// const float Score = (Dot * 1000.f) - Distance;




//new 5_13 OverDrive KillStreak
void ULGSOverDriveComponent::RegisterMeleeKill()
{
	if (!GetWorld())
	{
		return;
	}

	if (bOverDriveActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OD] RegisterMeleeKill ignored - OverDrive already active"));
		return;
	}

	MeleeKillStreakCount++;

	GetWorld()->GetTimerManager().ClearTimer(Timer_MeleeKillStreakWindow);
	GetWorld()->GetTimerManager().SetTimer(
		Timer_MeleeKillStreakWindow,
		this,
		&ULGSOverDriveComponent::ResetMeleeKillStreak,
		MeleeKillStreakWindow,
		false
	);

	UE_LOG(LogTemp, Warning, TEXT("[OD] Melee Kill Registered Count=%d / %d"),
		MeleeKillStreakCount,
		KillsForOverDrive);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.5f,
			FColor::Orange,
			FString::Printf(TEXT("[OD] Melee Kill %d / %d"), MeleeKillStreakCount, KillsForOverDrive)
		);
	}

	if (MeleeKillStreakCount >= KillsForOverDrive)
	{
		OverDriveCharge = OverDriveMaxCharge;
		ActivateOverDrive();

		GetWorld()->GetTimerManager().ClearTimer(Timer_MeleeKillStreakWindow);
		MeleeKillStreakCount = 0;
	}
}

void ULGSOverDriveComponent::ResetMeleeKillStreak()
{
	if (bOverDriveActive)
	{
		return;
	}

	MeleeKillStreakCount = 0;

	UE_LOG(LogTemp, Warning, TEXT("[OD] Melee Kill Streak Reset"));
}



float ULGSOverDriveComponent::GetOverDrivePercent() const
{
	if (OverDriveMaxCharge <= 0.f)
	{
		return 0.f;
	}

	return FMath::Clamp(OverDriveCharge / OverDriveMaxCharge, 0.f, 1.f);
}

void ULGSOverDriveComponent::AddOverDriveCharge(float DamageAmount)
{
	if (DamageAmount <= 0.f || OverDriveMaxCharge <= 0.f)
	{
		return;
	}

	const float ChargeToAdd = DamageAmount * DamageToChargeMultiplier;

	OverDriveCharge = FMath::Clamp(
		OverDriveCharge + ChargeToAdd,
		0.f,
		OverDriveMaxCharge
	);

	UE_LOG(LogTemp, Warning, TEXT("[OD] Charge Added Damage=%.1f Charge=%.1f / %.1f Percent=%.2f Ready=%d"),
		DamageAmount,
		OverDriveCharge,
		OverDriveMaxCharge,
		GetOverDrivePercent(),
		IsOverDriveReady() ? 1 : 0);
}

void ULGSOverDriveComponent::ResetOverDrive()
{
	OverDriveCharge = 0.f;
	bOverDriveActive = false;

	UE_LOG(LogTemp, Warning, TEXT("[OD] Reset"));

	//new 5_13 ODKillingStreak
	MeleeKillStreakCount = 0;

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(Timer_MeleeKillStreakWindow);
	}
}

void ULGSOverDriveComponent::ActivateOverDrive()
{
	if (!IsOverDriveReady())
	{
		UE_LOG(LogTemp, Warning, TEXT("[OD] Activate blocked - not ready Charge=%.1f / %.1f"),
			OverDriveCharge,
			OverDriveThreshold);
		return;
	}

	bOverDriveActive = true;

	UE_LOG(LogTemp, Warning, TEXT("[OD] ACTIVATED"));
}

void ULGSOverDriveComponent::DeactivateOverDrive()
{
	bOverDriveActive = false;

	UE_LOG(LogTemp, Warning, TEXT("[OD] Deactivated"));

	//new 5_13 ODKillingStreak Cleanup
	MeleeKillStreakCount = 0;

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(Timer_MeleeKillStreakWindow);
	}
}

//new 5_15 EventHorizon
void ULGSOverDriveComponent::StartEventHorizonCharge()
{
	if (bChargingEventHorizon || !OwnerCharacter || !GetWorld())
	{
		return;
	}

	//start charge
	LockedEventHorizonTarget = FindBestEventHorizonTarget();

	if (LockedEventHorizonTarget)
	{
		SpawnEventHorizonLockFX(LockedEventHorizonTarget);
	}
	

	bChargingEventHorizon = true;
	EventHorizonChargeStartTime = GetWorld()->GetTimeSeconds();

	if (UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement())
	{
		CachedWalkSpeedBeforeEH = MoveComp->MaxWalkSpeed;
		MoveComp->MaxWalkSpeed *= EventHorizonChargeMoveSpeedMultiplier;
	}

	LockedEventHorizonTarget = FindBestEventHorizonTarget();

	const FString TargetName = LockedEventHorizonTarget
		? LockedEventHorizonTarget->GetName()
		: TEXT("None");

	DebugOD(FString::Printf(TEXT("[EH] Charging boundary | Target: %s"), *TargetName), FColor::Orange);
}

AActor* ULGSOverDriveComponent::FindBestEventHorizonTarget()
{
	if (!OwnerCharacter || !GetWorld())
	{
		return nullptr;
	}

	const FVector Origin = OwnerCharacter->GetActorLocation();
	const FVector Forward = OwnerCharacter->GetActorForwardVector();

	TArray<FOverlapResult> Hits;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	const bool bHit = GetWorld()->OverlapMultiByChannel(
		Hits,
		Origin,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(EventHorizonTargetSearchRadius),
		Params
	);

	if (!bHit)
	{
		return nullptr;
	}

	AActor* BestTarget = nullptr;
	float BestScore = -999999.f;

	for (FOverlapResult& Hit : Hits)
	{
		AActor* Candidate = Hit.GetActor();

		if (!Candidate || Candidate == OwnerCharacter)
		{
			continue;
		}

		const FVector ToTarget = Candidate->GetActorLocation() - Origin;
		const float Distance = ToTarget.Size();

		if (Distance <= KINDA_SMALL_NUMBER)
		{
			continue;
		}

		const FVector Direction = ToTarget.GetSafeNormal();
		const float Dot = FVector::DotProduct(Forward, Direction);
		const float SafeDot = FMath::Clamp(Dot, -1.f, 1.f);
		const float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(SafeDot));

		if (AngleDegrees > EventHorizonTargetMaxAngleDegrees)
		{
			continue;
		}

		const float Score = (Dot * 1000.f) - Distance;

		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

void ULGSOverDriveComponent::ReleaseEventHorizon()
{
	if (!bChargingEventHorizon || !OwnerCharacter || !GetWorld())
	{
		return;
	}

	bChargingEventHorizon = false;

	if (UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = CachedWalkSpeedBeforeEH;
	}

	const float HeldTime = GetWorld()->GetTimeSeconds() - EventHorizonChargeStartTime;

	if (HeldTime < EventHorizonMinChargeTime)
	{
		DebugOD(TEXT("[EH] Charge failed — boundary not crossed"), FColor::Silver);
		ClearEventHorizonTarget();
		return;
	}

	EEventHorizonTier Tier = EEventHorizonTier::Normal;

	if (bOverDriveActive)
	{
		const float Roll = FMath::FRand();

		if (Roll <= GodChance)
		{
			Tier = EEventHorizonTier::God;
		}
		else if (Roll <= GodChance + AverageChance)
		{
			Tier = EEventHorizonTier::Average;
		}
		else
		{
			Tier = EEventHorizonTier::Dud;
		}
	}

	ExecuteEventHorizon(Tier, LockedEventHorizonTarget, HeldTime);

	ClearEventHorizonTarget();
}

void ULGSOverDriveComponent::ClearEventHorizonTarget()
{
	if (ActiveEventHorizonLockFX)
	{
		ActiveEventHorizonLockFX->DestroyComponent();
		ActiveEventHorizonLockFX = nullptr;
	}

	LockedEventHorizonTarget = nullptr;
}

void ULGSOverDriveComponent::ExecuteEventHorizon(
	EEventHorizonTier Tier,
	AActor* Target,
	float HeldTime
)
{
	const float ChargeAlpha = FMath::Clamp(HeldTime / EventHorizonMaxChargeTime, 0.f, 1.f);

	float Damage = 45.f;
	float Knockback = 600.f;
	UNiagaraSystem* FX = EventHorizonNormalFX;
	FColor Color = FColor::Cyan;
	FString Message = TEXT("[EH] Event Horizon released");

	switch (Tier)
	{
	case EEventHorizonTier::God:
		Damage = 180.f;
		Knockback = 2200.f;
		FX = EventHorizonGodFX;
		Color = FColor::Red;
		Message = TEXT("[EH] GOD EVENT HORIZON — something wonderful is going to happen");
		break;

	case EEventHorizonTier::Average:
		Damage = 110.f;
		Knockback = 1400.f;
		FX = EventHorizonAverageFX;
		Color = FColor::Orange;
		Message = TEXT("[EH] Event Horizon — Collapse Vector");
		break;

	case EEventHorizonTier::Dud:
		Damage = 65.f;
		Knockback = 850.f;
		FX = EventHorizonDudFX;
		Color = FColor::Silver;
		Message = TEXT("[EH] Dud Horizon — still hurts");
		break;

	default:
		break;
	}

	Damage *= FMath::Lerp(0.75f, 1.25f, ChargeAlpha);

	DebugOD(Message, Color);

	SpawnEventHorizonFX(FX, Target);
	ApplyEventHorizonDamage(Target, Damage, Knockback);

	if (bOverDriveActive)
	{
		SpawnRandomODSwingTrail();
		TryODRupture(Target);
	}
}

void ULGSOverDriveComponent::SpawnEventHorizonLockFX(AActor* Target)
{
	if (!Target || !EventHorizonLockOnFX)
	{
		return;
	}

	USceneComponent* AttachComp = Target->GetRootComponent();
	if (!AttachComp)
	{
		return;
	}

	ActiveEventHorizonLockFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
		EventHorizonLockOnFX,
		AttachComp,
		NAME_None,
		FVector(0.f, 0.f, 120.f),
		FRotator::ZeroRotator,
		EAttachLocation::KeepRelativeOffset,
		true
	);

	DebugOD(TEXT("[EH] TARGET LOCKED — boundary acquired"), FColor::Red, 1.2f);
}

void ULGSOverDriveComponent::SpawnEventHorizonFX(UNiagaraSystem* FX, AActor* Target)
{
	if (!FX || !GetWorld())
	{
		return;
	}

	const FVector SpawnLocation = Target
		? Target->GetActorLocation()
		: OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * 150.f;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		FX,
		SpawnLocation,
		OwnerCharacter ? OwnerCharacter->GetActorRotation() : FRotator::ZeroRotator
	);
}

void ULGSOverDriveComponent::ApplyEventHorizonDamage(AActor* Target, float Damage, float Knockback)
{
	if (!Target || !OwnerCharacter)
	{
		DebugOD(TEXT("[EH] Released with no target — boundary found nothing"), FColor::Silver);
		return;
	}

	UGameplayStatics::ApplyDamage(
		Target,
		Damage,
		OwnerCharacter->GetController(),
		OwnerCharacter,
		UDamageType::StaticClass()
	);

	//new 5_15 fix Warning
	if (UPrimitiveComponent* Primitive =
	Cast<UPrimitiveComponent>(Target->GetRootComponent()))
	{
		if (Primitive->IsSimulatingPhysics())
		{
			const FVector PushDir =
				(Target->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();

			Primitive->AddImpulse(PushDir * Knockback, NAME_None, true);
		}
	}
}

//new 5_15 important so quick click swings melee not EH
float ULGSOverDriveComponent::GetEventHorizonHeldTime() const
{
	if (!GetWorld() || !bChargingEventHorizon)
	{
		return 0.f;
	}

	return GetWorld()->GetTimeSeconds() - EventHorizonChargeStartTime;
}

void ULGSOverDriveComponent::CancelEventHorizonCharge()
{
	bChargingEventHorizon = false;

	ClearEventHorizonTarget();

	if (OwnerCharacter)
	{
		OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed =
			CachedWalkSpeedBeforeEH;
	}

	DebugOD(TEXT("[EH] Charge cancelled"), FColor::Silver, 1.5f);
}

//new 5_15 OD Swing Trail Pool
void ULGSOverDriveComponent::SpawnRandomODSwingTrail()
{
	if (!OwnerCharacter || ODMeleeSwingFXPool.Num() <= 0)
	{
		return;
	}

	const int32 Index = FMath::RandRange(0, ODMeleeSwingFXPool.Num() - 1);
	UNiagaraSystem* PickedFX = ODMeleeSwingFXPool[Index];

	if (!PickedFX)
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		PickedFX,
		OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * 120.f,
		OwnerCharacter->GetActorRotation()
	);
}

//new 5_15_ 20% OD Proc Rupture
void ULGSOverDriveComponent::TryODRupture(AActor* Target)
{
	if (!bOverDriveActive || !Target || !GetWorld())
	{
		return;
	}

	if (FMath::FRand() > ODEnemyRuptureChance)
	{
		return;
	}

	DebugOD(TEXT("[OD] GOD BLESSING — enemy rupture invoked"), FColor::Red, 2.f);

	if (ODRuptureFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ODRuptureFX,
			Target->GetActorLocation()
		);
	}

	UGameplayStatics::ApplyRadialDamage(
		GetWorld(),
		75.f,
		Target->GetActorLocation(),
		450.f,
		UDamageType::StaticClass(),
		{},
		OwnerCharacter,
		OwnerCharacter ? OwnerCharacter->GetController() : nullptr,
		true
	);
}

//5_15 tiny helper
void ULGSOverDriveComponent::DebugOD(const FString& Message, const FColor& Color, float Time)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Time, Color, Message);
	}

	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
}


#include "LGSAirWalkComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "LGSCoreJan12026Character.h"

ULGSAirWalkComponent::ULGSAirWalkComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULGSAirWalkComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	CachedMoveComp = OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr;

	NormalGravityScale = CachedMoveComp ? CachedMoveComp->GravityScale : 1.0f;
	AirWalkEnergyCurrent = AirWalkEnergyMax;
}

void ULGSAirWalkComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateAirWalk(DeltaTime);
}

void ULGSAirWalkComponent::HandlePress()
{
	if (!bHasAirWalkStrand || !OwnerCharacter)
	{
		return;
	}

	bAirWalkHeld = true;
	AirWalkHoldTime = 0.f;
	bApexRollConsumed = false;
}

void ULGSAirWalkComponent::HandleRelease()
{
	if (!bHasAirWalkStrand || !OwnerCharacter)
	{
		return;
	}

	const bool bWasLifting = bAirLiftActive;
	const float HeldTime = AirWalkHoldTime;

	bAirWalkHeld = false;
	AirWalkHoldTime = 0.f;

	if (bWasLifting)
	{
		EndAirLift(false);
		FallGracefullyWithVelocityChanger();
		EvaluateAirWalkApexRNG();
		return;
	}

	if (HeldTime < HoldThreshold)
	{
		PerformAirWalkTap();
	}
}

void ULGSAirWalkComponent::PerformAirWalkTap()
{
	if (!OwnerCharacter || !CachedMoveComp)
	{
		return;
	}

	if (ALGSCoreJan12026Character* LGSChar = Cast<ALGSCoreJan12026Character>(OwnerCharacter))
	{
		LGSChar->UnCrouch();
		LGSChar->CancelSprintForAirWalk();
	}

	const bool bIsFallingNow = CachedMoveComp->IsFalling();
	const float UseImpulse = bIsFallingNow ? TapAirWalkImpulseAir : TapAirWalkImpulseGround;

	OwnerCharacter->LaunchCharacter(FVector(0.f, 0.f, UseImpulse), false, true);

	AirWalkState = EAirWalkState::TapRise;
	bApexRollConsumed = false;
}

void ULGSAirWalkComponent::UpdateAirWalk(float DeltaSeconds)
{
	if (!CachedMoveComp)
	{
		return;
	}

	if (bAirWalkHeld && !bAirLiftActive)
	{
		AirWalkHoldTime += DeltaSeconds;

		if (AirWalkHoldTime >= HoldThreshold)
		{
			BeginAirLift();
		}
	}

	if (bAirLiftActive)
	{
		if (AirWalkEnergyCurrent <= 0.f)
		{
			EndAirLift(true);
			FallGracefullyWithVelocityChanger();
			EvaluateAirWalkApexRNG();
			return;
		}

		AirWalkEnergyCurrent = FMath::Max(0.f, AirWalkEnergyCurrent - LiftEnergyDrainPerSecond * DeltaSeconds);

		FVector V = CachedMoveComp->Velocity;
		V.Z = FMath::Min(V.Z + (LiftAccelerationZ * DeltaSeconds), LiftMaxUpVelocity);
		CachedMoveComp->Velocity = V;
		CachedMoveComp->GravityScale = LiftGravityScale;
	}

	if (!bAirLiftActive && !bApexRollConsumed && CachedMoveComp->IsFalling())
	{
		const float AbsZ = FMath::Abs(CachedMoveComp->Velocity.Z);
		if (AbsZ <= ApexVelocityThreshold)
		{
			EvaluateAirWalkApexRNG();
		}
	}
}

void ULGSAirWalkComponent::BeginAirLift()
{
	if (ALGSCoreJan12026Character* LGSChar = Cast<ALGSCoreJan12026Character>(OwnerCharacter))
	{
		LGSChar->UnCrouch();
		LGSChar->CancelSprintForAirWalk();
	}

	bAirLiftActive = true;
	AirWalkState = EAirWalkState::Lift;
	CachedMoveComp->GravityScale = LiftGravityScale;
}

void ULGSAirWalkComponent::EndAirLift(bool bFromEnergyDepletion)
{
	if (!bAirLiftActive || !CachedMoveComp)
	{
		return;
	}

	bAirLiftActive = false;
	AirWalkState = EAirWalkState::GracefulFall;
	CachedMoveComp->GravityScale = GracefulFallGravityScale;
}

void ULGSAirWalkComponent::FallGracefullyWithVelocityChanger()
{
	if (!CachedMoveComp)
	{
		return;
	}

	FVector V = CachedMoveComp->Velocity;

	if (V.Z < -600.f)
	{
		V.Z = -600.f;
	}

	CachedMoveComp->Velocity = V;
	CachedMoveComp->GravityScale = GracefulFallGravityScale;
}

void ULGSAirWalkComponent::EvaluateAirWalkApexRNG()
{
	if (bApexRollConsumed || !OwnerCharacter)
	{
		return;
	}

	bApexRollConsumed = true;

	const float Roll = FMath::FRand();

	if (Roll <= GodAirBoostChance)
	{
		bGodAirBoostAvailable = true;
		OwnerCharacter->LaunchCharacter(FVector(0.f, 0.f, GodAirBoostImpulse), false, true);

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(Timer_GodAirBoostReset);
			World->GetTimerManager().SetTimer(
				Timer_GodAirBoostReset,
				this,
				&ULGSAirWalkComponent::ResetGodAirBoost,
				0.35f,
				false
			);
		}
	}
}

void ALGSCoreJan12026Character::CancelSprintForAirWalk()
{
	bSprintHeld = false;
	UpdateSprintState();

	UE_LOG(LogTemplateCharacter, Warning, TEXT("[MOVE] Sprint canceled for AirWalk"));
}

void ULGSAirWalkComponent::HandleLanded(const FHitResult& Hit)
{
	if (CachedMoveComp)
	{
		CachedMoveComp->GravityScale = NormalGravityScale;
	}

	bAirLiftActive = false;
	bAirWalkHeld = false;
	AirWalkHoldTime = 0.f;
	bApexRollConsumed = false;
	AirWalkState = EAirWalkState::None;
	bGodAirBoostAvailable = false;

	AirWalkEnergyCurrent = FMath::Min(AirWalkEnergyMax, AirWalkEnergyCurrent + 15.f);
}

void ULGSAirWalkComponent::ResetGodAirBoost()
{
	bGodAirBoostAvailable = false;
}
#include "LGSAirWalkComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
//new 4_15 AirwalkUpdates
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
//end 4_15
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

//new 4_15 airwalk
void ULGSAirWalkComponent::TriggerSpaceTimeUpheaval()
{
	if (!OwnerCharacter || !CachedMoveComp)
	{
		return;
	}

	FVector V = CachedMoveComp->Velocity;

	// Hard-set or boost upward speed
	V.Z = FMath::Max(V.Z + SpaceTimeUpheavalImpulse, SpaceTimeUpheavalMaxZOverride);
	CachedMoveComp->Velocity = V;

	// Optional: lighter gravity during the burst
	CachedMoveComp->GravityScale = LiftGravityScale;

	bSpaceTimeUpheavalActive = true;

	if (SpaceTimeUpheavalFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			SpaceTimeUpheavalFX,
			OwnerCharacter->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			0.75f,
			FColor::Cyan,
			TEXT("[AIR] SPACE-TIME UPHEAVAL")
		);
	}
}

void ULGSAirWalkComponent::ConsumeDistortion(const FVector& InputDir)
{
	if (!OwnerCharacter || !CachedMoveComp || DistortionsRemaining <= 0)
	{
		return;
	}

	const bool bIsInitial = (DistortionsRemaining == MaxDistortions);
	const float UpImpulse = bIsInitial ? InitialUpImpulse : DistortionUpImpulse;

	FVector Launch = FVector(0.f, 0.f, UpImpulse);

	if (!InputDir.IsNearlyZero())
	{
		Launch += InputDir * HorizontalImpulse;
	}

	OwnerCharacter->LaunchCharacter(Launch, true, true);

	DistortionsRemaining--;
	TriggerWobble();
}

void ULGSAirWalkComponent::ApplyAirWalkDirectionalFeel(float DeltaSeconds)
{
	if (!OwnerCharacter || !CachedMoveComp)
	{
		return;
	}

	const FVector InputDir = GetInputDir();
	if (InputDir.IsNearlyZero())
	{
		return;
	}

	FVector Vel = CachedMoveComp->Velocity;
	const bool bRising = Vel.Z > 0.f;

	const float PushStrength = bRising ? RisingDirectionalPush : FallingDirectionalPush;
	const float CounterDrag = bRising ? RisingCounterDrag : FallingCounterDrag;

	// Quick horizontal push in input direction
	Vel += InputDir * PushStrength * DeltaSeconds;

	// Slight resistance / pushback so it feels like fighting unstable gravity
	Vel.X *= CounterDrag;
	Vel.Y *= CounterDrag;

	// Clamp horizontal speed so it stays controlled
	FVector HorizontalVel(Vel.X, Vel.Y, 0.f);
	const float HorizontalSpeed = HorizontalVel.Size();

	if (HorizontalSpeed > MaxAirWalkHorizontalSpeed)
	{
		const FVector Clamped = HorizontalVel.GetSafeNormal() * MaxAirWalkHorizontalSpeed;
		Vel.X = Clamped.X;
		Vel.Y = Clamped.Y;
	}

	CachedMoveComp->Velocity = Vel;
}






FVector ULGSAirWalkComponent::GetInputDir() const
{
	const ALGSCoreJan12026Character* LGSChar = Cast<ALGSCoreJan12026Character>(OwnerCharacter);
	if (!LGSChar)
	{
		return FVector::ZeroVector;
	}

	const FVector2D MoveInput = LGSChar->GetLastMoveInput();
	if (MoveInput.IsNearlyZero())
	{
		return FVector::ZeroVector;
	}

	const FRotator ControlRot = LGSChar->GetControlRotation();
	const FRotator YawOnlyRot(0.f, ControlRot.Yaw, 0.f);

	const FVector Fwd = FRotationMatrix(YawOnlyRot).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawOnlyRot).GetUnitAxis(EAxis::Y);

	FVector Dir = (Fwd * MoveInput.Y) + (Right * MoveInput.X);
	Dir.Z = 0.f;

	return Dir.GetClampedToMaxSize(1.f);
}



void ULGSAirWalkComponent::EnterAirWalk()
{
	bAirWalkActive = true;
	DistortionsRemaining = MaxDistortions;
}

void ULGSAirWalkComponent::TriggerWobble()
{
	auto* Char = GetOwnerCharacter();
	if (!Char) return;

	FVector Vel = Char->GetVelocity();
	Vel.Z *= 0.85f;

	Char->GetCharacterMovement()->Velocity = Vel;
}


//end 4_15

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

	//consider this
	// if (bAirLiftActive || AirWalkState == EAirWalkState::TapRise || AirWalkState == EAirWalkState::GracefulFall)
	// {
	// 	ApplyAirWalkDirectionalFeel(DeltaSeconds);
	// }
	//
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

// Or Consider This Version
// void ULGSAirWalkComponent::HandleLanded(const FHitResult& Hit)
// {
// 	if (bLandingChargeArmed)
// 	{
// 		ConsumeLandingCharge(Hit);
// 	}
//
// 	if (CachedMoveComp)
// 	{
// 		CachedMoveComp->GravityScale = NormalGravityScale;
// 	}
//
// 	bAirLiftActive = false;
// 	bAirWalkHeld = false;
// 	AirWalkHoldTime = 0.f;
// 	bApexRollConsumed = false;
// 	AirWalkState = EAirWalkState::None;
// 	bGodAirBoostAvailable = false;
// 	bAirWalkActive = false;
// 	DistortionsRemaining = 0;
//
// 	AirWalkEnergyCurrent = FMath::Min(AirWalkEnergyMax, AirWalkEnergyCurrent + 15.f);
// }


//isn't this part of the original Apex which we have to delete?
void ULGSAirWalkComponent::ResetGodAirBoost()
{
	bGodAirBoostAvailable = false;
}


// my new method :)
// void ULGSAirWalkComponent::engangeRGSSYstemToDeterminAirwalDistortions()
// {
// 	initial burst always happens
// 	extra distortions vary
//
// 	if (Roll <= GodChance)
// 	{
// 		DistortionsRemaining = 2; // total feel = entry + 2 extras
// 	}
// 	else if (Roll <= GodChance + AverageChance)
// 	{
// 		DistortionsRemaining = 1; // entry + 1 extra
// 	}
// 	else
// 	{
// 		DistortionsRemaining = 0; // entry only
// 	}
// }
//end 


//for super crazy: OwnerCharacter->LaunchCharacter(FVector(0.f, 0.f, SpaceTimeUpheavalImpulse), false, true);
//
//even crazier protects stronger upward momentum
//  V.Z = FMath::Max(V.Z, SpaceTimeUpheavalVerticalVelocity);
//

//for mild version
//  FVector V = CachedMoveComp->Velocity;
//  V.Z = SpaceTimeUpheavalMaxZOverride;
//  CachedMoveComp->Velocity = V;
//


///////////////////////Charged Melee Section Phase2/////////////

//4_14_Charged Melee Section for the next
//
// void ULGSAirWalkComponent::ArmLandingCharge()
// {
// 	bLandingChargeArmed = true;
//
// 	if (LandingChargeArmedFX && OwnerCharacter)
// 	{
// 		UNiagaraFunctionLibrary::SpawnSystemAttached(
// 			LandingChargeArmedFX,
// 			OwnerCharacter->GetRootComponent(),
// 			NAME_None,
// 			FVector::ZeroVector,
// 			FRotator::ZeroRotator,
// 			EAttachLocation::KeepRelativeOffset,
// 			true
// 		);
// 	}
//
// 	if (GEngine)
// 	{
// 		GEngine->AddOnScreenDebugMessage(
// 			-1,
// 			0.5f,
// 			FColor::Red,
// 			TEXT("[AIR] CHARGE ARMED")
// 		);
// 	}
// }

// AActor* ULGSAirWalkComponent::FindNearestLandingTarget(float Radius) const
// {
// 	if (!OwnerCharacter || !GetWorld())
// 	{
// 		return nullptr;
// 	}
//
// 	AActor* BestTarget = nullptr;
// 	float BestDistSq = Radius * Radius;
//
// 	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
// 	{
// 		AActor* Candidate = *It;
// 		if (!Candidate || Candidate == OwnerCharacter)
// 		{
// 			continue;
// 		}
//
// 		// Replace this with your actual enemy check
// 		if (!Candidate->ActorHasTag(TEXT("AI")))
// 		{
// 			continue;
// 		}
//
// 		const float DistSq = FVector::DistSquared(
// 			OwnerCharacter->GetActorLocation(),
// 			Candidate->GetActorLocation()
// 		);
//
// 		if (DistSq < BestDistSq)
// 		{
// 			BestDistSq = DistSq;
// 			BestTarget = Candidate;
// 		}
// 	}
//
// 	return BestTarget;
// }
//
// void ULGSAirWalkComponent::ConsumeLandingCharge(const FHitResult& Hit)
// {
// 	bLandingChargeArmed = false;
//
// 	if (!OwnerCharacter)
// 	{
// 		return;
// 	}
//
// 	AActor* Target = FindNearestLandingTarget(LandingChargeSearchRadius);
//
// 	if (bFaceNearestTargetOnLanding && Target)
// 	{
// 		FVector ToTarget = Target->GetActorLocation() - OwnerCharacter->GetActorLocation();
// 		ToTarget.Z = 0.f;
//
// 		if (!ToTarget.IsNearlyZero())
// 		{
// 			const FRotator FaceRot = ToTarget.Rotation();
// 			OwnerCharacter->SetActorRotation(FaceRot);
// 		}
// 	}
//
// 	if (ALGSCoreJan12026Character* LGSChar = Cast<ALGSCoreJan12026Character>(OwnerCharacter))
// 	{
// 		if (ULGSCombatCoreComponent* Combat = LGSChar->FindComponentByClass<ULGSCombatCoreComponent>())
// 		{
// 			// First pass: special landing burst function you add in CombatCore
// 			Combat->ExecuteLandingChargeBurst(Target, Hit.ImpactPoint);
// 		}
// 	}
// }

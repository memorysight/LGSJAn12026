#include "LGSAirWalkComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
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

bool ULGSAirWalkComponent::CanStartFreshAirWalk() const
{
	if (!OwnerCharacter || !CachedMoveComp)
	{
		return false;
	}

	// Once a session has started, landing is required before a fresh restart.
	if (bMustLandBeforeReuse)
	{
		return false;
	}

	// New AirWalk sessions must begin from the ground.
	return CachedMoveComp->IsMovingOnGround();
}

bool ULGSAirWalkComponent::IsDistortionCycleExhausted() const
{
	return bAirWalkActive
		&& bDistortionRollResolved
		&& DistortionsRemaining <= 0
		&& !bAirLiftActive;
}

void ULGSAirWalkComponent::HandlePress()
{
	if (!bHasAirWalkStrand || !OwnerCharacter || !CachedMoveComp)
	{
		return;
	}

	// Once the cycle is spent, no fresh attempts until landing.
	if (IsDistortionCycleExhausted())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				0.6f,
				FColor::Yellow,
				TEXT("[AIR] Distortion cycle exhausted - land to reset")
			);
		}
		return;
	}

	// Block midair restart attempts.
	if (!bAirWalkActive && !CanStartFreshAirWalk())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				0.6f,
				FColor::Yellow,
				TEXT("[AIR] Must land before starting AirWalk again")
			);
		}
		return;
	}

	bAirWalkHeld = true;
	AirWalkHoldTime = 0.f;
	LastLiftDuration = 0.f;
}

void ULGSAirWalkComponent::HandleRelease()
{
	if (!bHasAirWalkStrand || !OwnerCharacter || !CachedMoveComp)
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
		ApplyReleaseWobble();
		FallGracefullyWithVelocityChanger();

		const bool bCommitted = (LastLiftDuration >= MinCommitTime);

		if (bCommitted && !bDistortionRollResolved)
		{
			DetermineDistortionCountFromRoll();
			SpawnUpheavalRollFX(DistortionsRemaining);
		}

		return;
	}

	if (HeldTime < HoldThreshold)
	{
		const FVector InputDir = GetInputDir();

		if (IsDistortionCycleExhausted())
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					0.6f,
					FColor::Yellow,
					TEXT("[AIR] No more distortions - land to reset")
				);
			}
			return;
		}

		if (!bAirWalkActive)
		{
			EnterAirWalk();

			// EnterAirWalk can now fail if airborne / locked.
			if (!bAirWalkActive)
			{
				return;
			}

			ConsumeDistortion(InputDir);
			AirWalkState = EAirWalkState::TapRise;
			return;
		}

		if (CanUseExtraDistortion())
		{
			ConsumeDistortion(InputDir);
			AirWalkState = EAirWalkState::TapRise;
			return;
		}

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				0.5f,
				FColor::Yellow,
				TEXT("[AIR] Quick tap ignored - no extra distortions")
			);
		}
	}
}

void ULGSAirWalkComponent::EndAirLift(bool bFromEnergyDepletion)
{
	if (!CachedMoveComp)
	{
		return;
	}

	bAirLiftActive = false;
	AirWalkState = EAirWalkState::GracefulFall;

	FVector V = CachedMoveComp->Velocity;
	if (V.Z > 0.f)
	{
		V.Z *= 0.35f;
	}
	CachedMoveComp->Velocity = V;

	CachedMoveComp->GravityScale = GracefulFallGravityScale;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			0.5f,
			bFromEnergyDepletion ? FColor::Orange : FColor::Yellow,
			bFromEnergyDepletion
				? TEXT("[AIR] Lift ended - depleted")
				: TEXT("[AIR] Lift ended")
		);
	}
}

void ULGSAirWalkComponent::EnterAirWalk()
{
	if (bAirWalkActive)
	{
		return;
	}

	// Fresh midair re-entry is not allowed.
	if (!CanStartFreshAirWalk())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				0.6f,
				FColor::Yellow,
				TEXT("[AIR] Fresh AirWalk blocked - land first")
			);
		}
		return;
	}

	bAirWalkActive = true;
	bMustLandBeforeReuse = true;
	bEntryDistortionConsumed = false;
	bDistortionRollResolved = false;
	DistortionsRemaining = 0;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			0.75f,
			FColor::Green,
			TEXT("[AIR] EnterAirWalk")
		);
	}
}

bool ULGSAirWalkComponent::CanUseExtraDistortion() const
{
	return bAirWalkActive && DistortionsRemaining > 0;
}

void ULGSAirWalkComponent::DetermineDistortionCountFromRoll()
{
	if (bDistortionRollResolved)
	{
		return;
	}

	bDistortionRollResolved = true;

	const float Roll = FMath::FRand();

	if (Roll <= DudDistortionChance)
	{
		DistortionsRemaining = 1;
	}
	else if (Roll <= DudDistortionChance + AverageDistortionChance)
	{
		DistortionsRemaining = 2;
	}
	else
	{
		DistortionsRemaining = 3;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.2f,
			FColor::Green,
			FString::Printf(TEXT("[AIR] Extra Distortions Rolled: %d"), DistortionsRemaining)
		);
	}
}

void ULGSAirWalkComponent::ConsumeDistortion(const FVector& InputDir)
{
	if (!OwnerCharacter || !CachedMoveComp)
	{
		return;
	}

	const bool bInitialEntry = !bEntryDistortionConsumed;
	const float UpImpulse = bInitialEntry ? InitialUpImpulse : DistortionUpImpulse;

	if (bInitialEntry)
	{
		bEntryDistortionConsumed = true;
	}
	else
	{
		if (DistortionsRemaining <= 0)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					0.5f,
					FColor::Yellow,
					TEXT("[AIR] No distortions remaining")
				);
			}
			return;
		}

		DistortionsRemaining--;
	}

	FVector Launch = FVector(0.f, 0.f, UpImpulse);

	if (!InputDir.IsNearlyZero())
	{
		Launch += InputDir * HorizontalImpulse;
	}

	OwnerCharacter->LaunchCharacter(Launch, true, true);
	TriggerWobble();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			0.6f,
			FColor::Cyan,
			FString::Printf(TEXT("[AIR] Distortion used. Remaining=%d"), DistortionsRemaining)
		);
	}
}

void ULGSAirWalkComponent::TriggerWobble()
{
	ACharacter* Char = GetOwnerCharacter();
	if (!Char || !Char->GetCharacterMovement())
	{
		return;
	}

	FVector Vel = Char->GetVelocity();
	Vel.Z *= 0.85f;
	Char->GetCharacterMovement()->Velocity = Vel;
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
}

void ULGSAirWalkComponent::BeginAirLift()
{
	if (bAirLiftActive || !OwnerCharacter || !CachedMoveComp)
	{
		return;
	}

	// Held lift is only for the initial commitment phase.
	// After the roll is resolved, the player must land before starting again.
	if (bDistortionRollResolved)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				0.6f,
				FColor::Yellow,
				TEXT("[AIR] Hold blocked - land to start a new cycle")
			);
		}
		return;
	}

	if (!bAirWalkActive)
	{
		EnterAirWalk();

		// EnterAirWalk can fail now if airborne / locked.
		if (!bAirWalkActive)
		{
			return;
		}
	}

	if (ALGSCoreJan12026Character* LGSChar = Cast<ALGSCoreJan12026Character>(OwnerCharacter))
	{
		LGSChar->UnCrouch();
		LGSChar->CancelSprintForAirWalk();
	}

	bAirLiftActive = true;
	AirWalkState = EAirWalkState::Lift;
	CachedMoveComp->GravityScale = LiftGravityScale;

	// Immediate kick for held lift start
	FVector V = CachedMoveComp->Velocity;
	V.Z = FMath::Max(V.Z, HoldLiftStartZ);
	CachedMoveComp->Velocity = V;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			0.75f,
			FColor::Cyan,
			FString::Printf(TEXT("[AIR] HOLD LIFT START Z=%.1f"), V.Z)
		);
	}
}

void ULGSAirWalkComponent::TriggerSpaceTimeUpheaval()
{
	if (!OwnerCharacter || !CachedMoveComp)
	{
		return;
	}

	if (!bAirWalkActive)
	{
		EnterAirWalk();

		if (!bAirWalkActive)
		{
			return;
		}
	}

	FVector V = CachedMoveComp->Velocity;
	V.Z = SpaceTimeUpheavalMaxZOverride;
	CachedMoveComp->Velocity = V;
	CachedMoveComp->GravityScale = SpaceTimeUpheavalGravityScale;

	bSpaceTimeUpheavalActive = true;
	bAirLiftActive = false;
	AirWalkState = EAirWalkState::Lift;

	DetermineDistortionCountFromRoll();

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
			FString::Printf(TEXT("[AIR] SPACE-TIME UPHEAVAL Z=%.1f"), V.Z)
		);
	}
}

void ULGSAirWalkComponent::SpawnUpheavalRollFX(int32 Count)
{
	UNiagaraSystem* FX = nullptr;

	if (Count == 1)
	{
		FX = DudUpheavalFX;
	}
	else if (Count == 2)
	{
		FX = AverageUpheavalFX;
	}
	else if (Count == 3)
	{
		FX = GodUpheavalFX;
	}

	if (!FX || !OwnerCharacter)
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAttached(
		FX,
		OwnerCharacter->GetRootComponent(),
		NAME_None,
		FVector(0.f, 0.f, 60.f),
		FRotator::ZeroRotator,
		EAttachLocation::KeepRelativeOffset,
		true
	);
}

void ULGSAirWalkComponent::ApplyReleaseWobble()
{
	if (!CachedMoveComp || !OwnerCharacter)
	{
		return;
	}

	FVector V = CachedMoveComp->Velocity;

	// Downward tug
	V.Z -= WobbleDownImpulse;

	// Small lateral instability
	const FVector RandXY = FVector(
		FMath::FRandRange(-1.f, 1.f),
		FMath::FRandRange(-1.f, 1.f),
		0.f
	).GetClampedToMaxSize(1.f);

	V += RandXY * WobbleLateralJitter;

	CachedMoveComp->Velocity = V;
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
		LastLiftDuration += DeltaSeconds;

		if (AirWalkEnergyCurrent <= 0.f)
		{
			EndAirLift(true);
			ApplyReleaseWobble();
			FallGracefullyWithVelocityChanger();
			return;
		}

		AirWalkEnergyCurrent = FMath::Max(
			0.f,
			AirWalkEnergyCurrent - LiftEnergyDrainPerSecond * DeltaSeconds
		);

		FVector V = CachedMoveComp->Velocity;
		V.Z = FMath::Min(
			V.Z + (LiftAccelerationZ * DeltaSeconds),
			LiftMaxUpVelocity
		);
		CachedMoveComp->Velocity = V;
		CachedMoveComp->GravityScale = LiftGravityScale;

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				0.f,
				FColor::Cyan,
				FString::Printf(TEXT("[AIR] HOLD ASCENT Z=%.1f Held=%.2f"), V.Z, LastLiftDuration)
			);
		}

		// Optional later:
		// ApplyAirWalkDirectionalFeel(DeltaSeconds);
	}

	// Optional later:
	// if (bAirLiftActive || AirWalkState == EAirWalkState::TapRise || AirWalkState == EAirWalkState::GracefulFall)
	// {
	//     ApplyAirWalkDirectionalFeel(DeltaSeconds);
	// }
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

	Vel += InputDir * PushStrength * DeltaSeconds;

	Vel.X *= CounterDrag;
	Vel.Y *= CounterDrag;

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

void ULGSAirWalkComponent::ResetAirWalkState()
{
	bAirLiftActive = false;
	bAirWalkHeld = false;
	AirWalkHoldTime = 0.f;
	AirWalkState = EAirWalkState::None;

	bAirWalkActive = false;
	bEntryDistortionConsumed = false;
	bDistortionRollResolved = false;
	DistortionsRemaining = 0;
	bMustLandBeforeReuse = false;
	bSpaceTimeUpheavalActive = false;

	LastLiftDuration = 0.f;
}

void ULGSAirWalkComponent::HandleLanded(const FHitResult& Hit)
{
	// Phase 2 later:
	// if (bLandingChargeArmed)
	// {
	// 	ConsumeLandingCharge(Hit);
	// }

	if (CachedMoveComp)
	{
		CachedMoveComp->GravityScale = NormalGravityScale;
	}

	ResetAirWalkState();

	AirWalkEnergyCurrent = FMath::Min(AirWalkEnergyMax, AirWalkEnergyCurrent + 15.f);
}
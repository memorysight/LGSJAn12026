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

void ULGSAirWalkComponent::HandlePress()
{
	if (!bHasAirWalkStrand || !OwnerCharacter)
	{
		return;
	}

	bAirWalkHeld = true;
	AirWalkHoldTime = 0.f;
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
		FallGracefullyWithVelocityChanger();
		return;
	}

	// Quick RMB = fast, snappy burst
	if (HeldTime < HoldThreshold)
	{
		const FVector InputDir = GetInputDir();

		// First entry into AirWalk chain
		if (!bAirWalkActive)
		{
			EnterAirWalk();
			ConsumeDistortion(InputDir);
			AirWalkState = EAirWalkState::TapRise;
			return;
		}

		// Later extra distortions
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

void ULGSAirWalkComponent::EnterAirWalk()
{
	if (bAirWalkActive)
	{
		return;
	}

	bAirWalkActive = true;
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

	if (ALGSCoreJan12026Character* LGSChar = Cast<ALGSCoreJan12026Character>(OwnerCharacter))
	{
		LGSChar->UnCrouch();
		LGSChar->CancelSprintForAirWalk();
	}

	if (!bAirWalkActive)
	{
		EnterAirWalk();
	}

	bAirLiftActive = true;
	AirWalkState = EAirWalkState::Lift;
	CachedMoveComp->GravityScale = LiftGravityScale;
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
	}

	FVector V = CachedMoveComp->Velocity;
	V.Z = SpaceTimeUpheavalMaxZOverride;
	CachedMoveComp->Velocity = V;
	CachedMoveComp->GravityScale = SpaceTimeUpheavalGravityScale;

	bSpaceTimeUpheavalActive = true;
	bAirLiftActive = false;
	AirWalkState = EAirWalkState::Lift;

	// Resolve the extra distortions here, after the committed hold/upheaval
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

			// First-pass committed hold: start the major ascent right away
			TriggerSpaceTimeUpheaval();
		}
	}

	if (bAirLiftActive)
	{
		if (AirWalkEnergyCurrent <= 0.f)
		{
			EndAirLift(true);
			FallGracefullyWithVelocityChanger();
			return;
		}

		AirWalkEnergyCurrent = FMath::Max(
			0.f,
			AirWalkEnergyCurrent - LiftEnergyDrainPerSecond * DeltaSeconds
		);

		FVector V = CachedMoveComp->Velocity;
		V.Z = FMath::Min(V.Z + (LiftAccelerationZ * DeltaSeconds), LiftMaxUpVelocity);
		CachedMoveComp->Velocity = V;
		CachedMoveComp->GravityScale = LiftGravityScale;
	}

	// Optional later:
	// if (bAirLiftActive || AirWalkState == EAirWalkState::TapRise || AirWalkState == EAirWalkState::GracefulFall)
	// {
	// 	ApplyAirWalkDirectionalFeel(DeltaSeconds);
	// }
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
	bSpaceTimeUpheavalActive = false;
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
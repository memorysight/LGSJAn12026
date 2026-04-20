#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LGSAirWalkComponent.generated.h"

class ACharacter;
class UNiagaraSystem;
class UCharacterMovementComponent;

UENUM(BlueprintType)
enum class EAirWalkState : uint8
{
	None         UMETA(DisplayName = "None"),
	TapRise      UMETA(DisplayName = "Tap Rise"),
	Lift         UMETA(DisplayName = "Lift"),
	GracefulFall UMETA(DisplayName = "Graceful Fall")
};

UCLASS(ClassGroup = (Movement), meta = (BlueprintSpawnableComponent))
class LGSCOREJAN12026_API ULGSAirWalkComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULGSAirWalkComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	void HandlePress();
	void HandleRelease();

	UFUNCTION(BlueprintPure, Category = "Movement|AirWalk")
	bool IsAirWalkActive() const
	{
		return bAirLiftActive
			|| AirWalkState == EAirWalkState::TapRise
			|| AirWalkState == EAirWalkState::GracefulFall;
	}

	UFUNCTION(BlueprintPure, Category = "Movement|AirWalk")
	bool HasAirWalkStrand() const { return bHasAirWalkStrand; }

	UFUNCTION(BlueprintCallable, Category = "Movement|AirWalk")
	void SetHasAirWalkStrand(bool bEnabled) { bHasAirWalkStrand = bEnabled; }

	UFUNCTION()
	void HandleLanded(const FHitResult& Hit);

	void EnterAirWalk();
	void ConsumeDistortion(const FVector& InputDir);
	void TriggerWobble();
	FVector GetInputDir() const;
	ACharacter* GetOwnerCharacter() const { return OwnerCharacter; }

	UFUNCTION(BlueprintCallable, Category="Movement|AirWalk")
	void TriggerSpaceTimeUpheaval();

protected:
	void BeginAirLift();
	void EndAirLift(bool bFromEnergyDepletion);
	void PerformAirWalkTap();
	void UpdateAirWalk(float DeltaSeconds);
	void FallGracefullyWithVelocityChanger();

	void ApplyAirWalkDirectionalFeel(float DeltaSeconds);

	void DetermineDistortionCountFromRoll();
	bool CanUseExtraDistortion() const;
	void ResetAirWalkState();

	// Locking helpers to prevent airborne re-entry until landing
	bool CanStartFreshAirWalk() const;
	bool IsDistortionCycleExhausted() const;

	// phase2
	void SpawnUpheavalRollFX(int32 Count);
	void ApplyReleaseWobble();
	// end

	ACharacter* OwnerCharacter = nullptr;
	UCharacterMovementComponent* CachedMoveComp = nullptr;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|AirWalk")
	bool bHasAirWalkStrand = true;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|AirWalk")
	bool bAirWalkHeld = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|AirWalk")
	bool bAirLiftActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|AirWalk")
	float AirWalkHoldTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|AirWalk")
	EAirWalkState AirWalkState = EAirWalkState::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|AirWalk")
	float TapAirWalkImpulseGround = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|AirWalk")
	float TapAirWalkImpulseAir = 850.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|AirWalk")
	float HoldThreshold = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|AirWalk")
	float LiftAccelerationZ = 2200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|AirWalk")
	float LiftMaxUpVelocity = 2800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|AirWalk")
	float LiftEnergyDrainPerSecond = 18.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|AirWalk")
	float AirWalkEnergyMax = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|AirWalk")
	float AirWalkEnergyCurrent = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|AirWalk")
	float GracefulFallGravityScale = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|AirWalk")
	float NormalGravityScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|AirWalk")
	float LiftGravityScale = 0.14f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Distortion")
	int32 MaxDistortions = 3;

	UPROPERTY(BlueprintReadOnly, Category="Movement|AirWalk|Distortion")
	int32 DistortionsRemaining = 0;

	UPROPERTY(BlueprintReadOnly, Category="Movement|AirWalk|Distortion")
	bool bAirWalkActive = false;

	UPROPERTY(BlueprintReadOnly, Category="Movement|AirWalk|Distortion")
	bool bEntryDistortionConsumed = false;

	UPROPERTY(BlueprintReadOnly, Category="Movement|AirWalk|Distortion")
	bool bDistortionRollResolved = false;

	// Once AirWalk starts, the player must land before starting a fresh cycle again.
	UPROPERTY(BlueprintReadOnly, Category="Movement|AirWalk|Distortion")
	bool bMustLandBeforeReuse = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Distortion")
	float DudDistortionChance = 0.20f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Distortion")
	float AverageDistortionChance = 0.60f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Distortion")
	float InitialUpImpulse = 1400.f;

	// phase 2
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|FX")
	UNiagaraSystem* DudUpheavalFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|FX")
	UNiagaraSystem* AverageUpheavalFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|FX")
	UNiagaraSystem* GodUpheavalFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Distortion")
	float DistortionUpImpulse = 850.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Distortion")
	float HorizontalImpulse = 750.f;

	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess="true"))
	FVector2D LastMoveInput = FVector2D::ZeroVector;

	UFUNCTION(BlueprintPure, Category="Movement")
	FVector2D GetLastMoveInput() const { return LastMoveInput; }

	UPROPERTY(BlueprintReadOnly, Category="Movement|AirWalk|Landing")
	bool bLandingChargeArmed = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Landing")
	float LandingChargeSearchRadius = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Landing")
	bool bFaceNearestTargetOnLanding = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Upheaval")
	float SpaceTimeUpheavalGravityScale = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Upheaval")
	float SpaceTimeUpheavalImpulse = 1600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Upheaval")
	float SpaceTimeUpheavalMaxZOverride = 2200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Upheaval")
	bool bSpaceTimeUpheavalActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|FX")
	UNiagaraSystem* AirWalkStartFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|FX")
	UNiagaraSystem* AirWalkLiftFX = nullptr;

	UPROPERTY(EditAnywhere, Category="AirWalk|Upheaval")
	float MinCommitTime = 0.35f;

	UPROPERTY(BlueprintReadOnly, Category="AirWalk|Upheaval")
	float LastLiftDuration = 0.f;

	UPROPERTY(EditAnywhere, Category="AirWalk|Feel")
	float WobbleDownImpulse = 220.f;

	UPROPERTY(EditAnywhere, Category="AirWalk|Feel")
	float WobbleLateralJitter = 120.f;

	// additional hold kick
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Upheaval")
	float HoldLiftStartZ = 1400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|FX")
	UNiagaraSystem* AirWalkLandingChargeFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|FX")
	UNiagaraSystem* SpaceTimeUpheavalFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Feel")
	float RisingDirectionalPush = 220.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Feel")
	float FallingDirectionalPush = 320.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Feel")
	float RisingCounterDrag = 0.92f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Feel")
	float FallingCounterDrag = 0.88f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Feel")
	float MaxAirWalkHorizontalSpeed = 900.f;
};
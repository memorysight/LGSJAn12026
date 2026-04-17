#pragma once 

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "LGSAirWalkComponent.generated.h"

class ACharacter;
//new 4_15 Airwalk Updates
class UNiagaraSystem;
//end 4_15
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

	// Character forwards input here
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

	//new 4_15 airwalk Updates
	void EnterAirWalk();
	void ConsumeDistortion(const FVector& InputDir);
	void TriggerWobble();
	FVector GetInputDir() const;
	ACharacter* GetOwnerCharacter() const { return OwnerCharacter; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Distortion")
	int32 MaxDistortions = 3;

	UPROPERTY(BlueprintReadOnly, Category="Movement|AirWalk|Distortion")
	int32 DistortionsRemaining = 0;

	UPROPERTY(BlueprintReadOnly, Category="Movement|AirWalk|Distortion")
	bool bAirWalkActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Distortion")
	float InitialUpImpulse = 1400.f;

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

	//4_17 AW Upheaval More Pronounced
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|Upheaval")
	float SpaceTimeUpheavalGravityScale = 0.15f;
	//end 4_17
	 UFUNCTION(BlueprintCallable, Category="Movement|AirWalk")
	 void TriggerSpaceTimeUpheaval();

	//end 4_15
	

protected:
	void BeginAirLift();
	void EndAirLift(bool bFromEnergyDepletion);
	void PerformAirWalkTap();
	void UpdateAirWalk(float DeltaSeconds);
	void EvaluateAirWalkApexRNG();
	void FallGracefullyWithVelocityChanger();
	void ResetGodAirBoost();

	ACharacter* OwnerCharacter = nullptr;
	UCharacterMovementComponent* CachedMoveComp = nullptr;
	

protected:

	//new 4_15 important directional dynamics for standing still airwalk
	void ApplyAirWalkDirectionalFeel(float DeltaSeconds);
	
	//below already defined but in the right scope?
	// FVector GetInputDir() const;
	//below already defined but in the right scope?
	// ACharacter* GetOwnerCharacter() const { return OwnerCharacter; }
	//end 4_14

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|AirWalk")
	bool bHasAirWalkStrand = true;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|AirWalk")
	bool bAirWalkHeld = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|AirWalk")
	bool bAirLiftActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|AirWalk")
	bool bGodAirBoostAvailable = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|AirWalk")
	bool bApexRollConsumed = false;

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
	float LiftAccelerationZ = 420.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|AirWalk")
	float LiftMaxUpVelocity = 500.f;

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
	float ApexVelocityThreshold = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|AirWalk")
	float GodAirBoostChance = 0.20f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|AirWalk")
	float GodAirBoostImpulse = 650.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|AirWalk")
	float LiftGravityScale = 0.35f;

	//new 4_15 Airwalk Updates
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|FX")
	UNiagaraSystem* AirWalkLandingChargeFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|AirWalk|FX")
	UNiagaraSystem* SpaceTimeUpheavalFX = nullptr;

	//new 4_15 Airwalk Updates Feeling like walking on air
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
	//end 4_15
	

	//new 4_15_Airwalk Charging dynamics for second pass
	// UFUNCTION(BlueprintCallable, Category="Movement|AirWalk|Landing")
	// void ArmLandingCharge();
	
	//  this below causes and error, already defined and conflicts
	// UPROPERTY(BlueprintReadOnly, Category="Movement|AirWalk|Landing")
	// bool bLandingChargeArmed = false;
	//
	// void ConsumeLandingCharge(const FHitResult& Hit);
	// AActor* FindNearestLandingTarget(float Radius) const;
	
	//end 4_14

	FTimerHandle Timer_GodAirBoostReset;
};
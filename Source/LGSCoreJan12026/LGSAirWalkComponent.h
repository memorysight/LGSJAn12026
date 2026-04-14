#pragma once 

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "LGSAirWalkComponent.generated.h"

class ACharacter;
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

	FTimerHandle Timer_GodAirBoostReset;
};
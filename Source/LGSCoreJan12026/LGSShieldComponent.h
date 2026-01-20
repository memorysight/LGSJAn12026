#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Templates/SubclassOf.h"
#include "LGSShieldComponent.generated.h"

class AActor;
class ACharacter;
class AController;
class UPrimitiveComponent;
class UNiagaraSystem;
class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS(ClassGroup=(Shield), meta=(BlueprintSpawnableComponent))
class LGSCOREJAN12026_API ULGSShieldComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULGSShieldComponent();

	// ---- Public API ----
	UFUNCTION(BlueprintCallable, Category="Shield")
	void ToggleShield();

	UFUNCTION(BlueprintCallable, Category="Shield")
	void ActivateShield();

	UFUNCTION(BlueprintCallable, Category="Shield")
	void DeactivateShield();

	UFUNCTION(BlueprintPure, Category="Shield")
	bool IsShieldActiveAndPowered() const { return bShieldActive && ShieldEnergy > 0.f && !bShieldBroken; }

	UFUNCTION(BlueprintCallable, Category="Shield|State")
	void GetShieldState(float& OutEnergy, float& OutMax, bool& OutActive, bool& OutBroken) const;

	UFUNCTION(BlueprintCallable, Category="Shield|State")
	void ApplyShieldState(float InEnergy, float InMax, bool bInActive, bool bInBroken);

	// Character calls this from TakeDamage override
	float HandleIncomingDamage(float DamageAmount, AController* EventInstigator, AActor* DamageCauser);

	// Optional convenience for logging / UI
	UFUNCTION(BlueprintPure, Category="Shield|State")
	float GetShieldEnergy() const { return ShieldEnergy; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ---- Overlap ----
	UFUNCTION()
	void OnShieldBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// ---- Internals ----
	void StartShieldDrain();
	void StopShieldDrain();
	void DrainTick();

	void StartShieldRegenWithDelay();
	void StartShieldRegen();
	void StopShieldRegen();
	void RegenTick();

	void BreakShield();
	void RefreshShieldVisualState();

	// Burst / TimeShift
	void TriggerShieldBurst();
	void UpdateShieldBurstEligibility();
	void MaybeStartBurstTimeShift(int32 NumEnemiesAffected, bool bWasGodBurst);
	void EndBurstTimeShift();

	ACharacter* GetOwningCharacter() const;

protected:
	// ---- References to Character-owned components (NOT spawned here) ----
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shield|Components", meta=(AllowPrivateAccess="true"))
	USceneComponent* ShieldRootComp = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shield|Components", meta=(AllowPrivateAccess="true"))
	USphereComponent* ShieldCollisionComp = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shield|Components", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* ShieldVisualComp = nullptr;

	// ---- Setup ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Setup")
	FName AttachSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Setup")
	float CollisionSphereRadius = 65.f;

	// ---- Core ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Config")
	float MaxShieldEnergy = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Runtime")
	float ShieldEnergy = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Config")
	float ShieldDrainPerSecond = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Config")
	float ShieldRegenPerSecond = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Config")
	float ShieldRegenDelay = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Config")
	bool bDestroyBulletsOnOverlap = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Config")
	TSubclassOf<AActor> BulletClass;

	// ---- Burst ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Burst")
	bool bShieldBurstEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Burst")
	float ShieldBurstMinChargePercent = 0.7f;

	UPROPERTY(BlueprintReadOnly, Category="Shield|Burst")
	bool bShieldBurstEligible = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Burst")
	float ShieldBurstRadius = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Burst")
	float ShieldBurstBaseDamage = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Burst")
	float ShieldBurstGodDamageMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Burst")
	float ShieldBurstDudDamageMultiplier = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Burst")
	float ShieldBurstKnockbackStrength = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Burst|FX")
	UNiagaraSystem* ShieldBurstAverageFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Burst|FX")
	UNiagaraSystem* ShieldBurstGodFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Burst|FX")
	UNiagaraSystem* ShieldBurstDudFX = nullptr;

	// ---- TimeShift ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Burst|TimeShift")
	bool bBurstTimeShiftEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Burst|TimeShift", meta=(ClampMin="0.01", ClampMax="1.0"))
	float BurstTimeShiftGlobalDilation = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Burst|TimeShift", meta=(ClampMin="0.1", ClampMax="2.0"))
	float BurstTimeShiftPlayerDilation = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Burst|TimeShift", meta=(ClampMin="0.05", ClampMax="2.0"))
	float BurstTimeShiftDuration = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Burst|TimeShift")
	int32 BurstTimeShiftMinEnemies = 2;

	UPROPERTY(BlueprintReadOnly, Category="Shield|Burst|TimeShift")
	int32 LastBurstEnemiesInRadius = 0;

	UPROPERTY(BlueprintReadOnly, Category="Shield|Burst|TimeShift")
	float LastBurstEnemyProximityScore = 0.f;

	// ---- State ----
	UPROPERTY(BlueprintReadOnly, Category="Shield|State")
	bool bShieldActive = false;

	UPROPERTY(BlueprintReadOnly, Category="Shield|State")
	bool bShieldBroken = false;

	// ---- Timers ----
	FTimerHandle Timer_ShieldDrain;
	FTimerHandle Timer_ShieldRegen;
	FTimerHandle Timer_ShieldRegenDelay;
	FTimerHandle Timer_BurstTimeShift;
};

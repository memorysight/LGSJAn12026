#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Templates/SubclassOf.h"
#include "TimerManager.h"
#include "LGSCombatCoreComponent.generated.h"

class UAnimMontage;
class AActor;
class ALGSCoreJan12026Projectile;

UENUM(BlueprintType)
enum class ECombatMode : uint8
{
	Ranged UMETA(DisplayName="Ranged"),
	Melee  UMETA(DisplayName="Melee")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatModeChanged, ECombatMode, NewMode);

//new 11_18RangedDepletion
// Ranged Energy MVP1 — HUD notification
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnRangedEnergyChanged,
	float, CurrentEnergy,
	float, MaxEnergy
);

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class LGSCOREJAN12026_API ULGSCombatCoreComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULGSCombatCoreComponent();

	// --- Mode ---
	UFUNCTION(BlueprintCallable, Category="Combat|Mode")
	void ToggleCombatMode();

	UFUNCTION(BlueprintCallable, Category="Combat|Mode")
	void SetCombatMode(ECombatMode NewMode);

	UFUNCTION(BlueprintPure, Category="Combat|Mode")
	ECombatMode GetCombatMode() const { return CombatMode; }

	UPROPERTY(BlueprintAssignable, Category="Combat|Mode")
	FOnCombatModeChanged OnCombatModeChanged;

	// --- Input intents (Character calls these) ---
	UFUNCTION(BlueprintCallable, Category="Combat|Input")
	void TryPrimary();

	UFUNCTION(BlueprintCallable, Category="Combat|Input")
	void TrySecondary();

	// --- Ranged config ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Ranged")
	TSubclassOf<ALGSCoreJan12026Projectile> BulletClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Ranged")
	TSubclassOf<ALGSCoreJan12026Projectile> HyperDriveBulletClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Ranged", meta=(ClampMin="0.0", ClampMax="1.0"))
	float HyperDriveBulletChance = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Ranged")
	FName MuzzleSocketName = TEXT("Muzzle");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Ranged")
	float FireCooldown = 0.10f;

	UFUNCTION(BlueprintPure, Category="Combat")
	bool IsRangedMode() const { return CombatMode == ECombatMode::Ranged; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Ranged", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAnimMontage> FP_Rifle_Shoot_Montage = nullptr;

	UFUNCTION(BlueprintCallable, Category="Combat|Input")
	void StartAutoFire();

	UFUNCTION(BlueprintCallable, Category="Combat|Input")
	void StopAutoFire();

	//new 11_18RangedDepletion
	// ============================================================
	// RANGED ENERGY MVP1
	// ============================================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Ranged|Energy",
		meta=(ClampMin="1.0"))
	float MaxRangedEnergy = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Ranged|Energy")
	float CurrentRangedEnergy = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Ranged|Energy",
		meta=(ClampMin="0.0"))
	float RegularShotEnergyCost = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Ranged|Energy",
		meta=(ClampMin="0.0"))
	float ExplosiveShotEnergyCost = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Ranged|Energy",
		meta=(ClampMin="0.0"))
	float EnergyRechargeDelay = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Ranged|Energy",
		meta=(ClampMin="0.0"))
	float EnergyRechargeRate = 20.0f;

	UPROPERTY(BlueprintAssignable, Category="Combat|Ranged|Energy")
	FOnRangedEnergyChanged OnRangedEnergyChanged;

	UFUNCTION(BlueprintPure, Category="Combat|Ranged|Energy")
	float GetRangedEnergyPercent() const
	{
		return MaxRangedEnergy > 0.0f
			? FMath::Clamp(CurrentRangedEnergy / MaxRangedEnergy, 0.0f, 1.0f)
			: 0.0f;
	}

	// --- Melee config ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Melee")
	UAnimMontage* MeleeMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Melee")
	float MeleeCooldown = 0.35f;

	UFUNCTION(BlueprintCallable, Category="Combat|Melee")
	void BeginMeleeDamage();

	UFUNCTION(BlueprintCallable, Category="Combat|Melee")
	void EndMeleeDamage();

	// --- Trace internals ---
	void StartMeleeTraceLoop();
	void StopMeleeTraceLoop();
	void PerformMeleeTrace();

	bool bMeleeDamageActive = false;

	TSet<TWeakObjectPtr<AActor>> HitActorsThisSwing;

	FTimerHandle Timer_MeleeTrace;

	UPROPERTY(EditAnywhere, Category="Combat|Melee|Trace")
	float MeleeTraceDistance = 200.f;

	UPROPERTY(EditAnywhere, Category="Combat|Melee|Trace")
	float MeleeTraceRadius = 35.f;

	UPROPERTY(EditAnywhere, Category="Combat|Melee|Trace")
	float MeleeDamage = 25.f;

	UPROPERTY(EditAnywhere, Category="Combat|Melee|Trace")
	TEnumAsByte<ECollisionChannel> MeleeTraceChannel = ECC_Pawn;

	UPROPERTY(EditAnywhere, Category="Combat|Melee|Trace")
	bool bDrawMeleeDebug = true;

	UPROPERTY(EditAnywhere, Category="Combat|Melee|Trace")
	FName MeleeTraceSocketName = TEXT("WeaponSocket_R");

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category="Combat|Mode")
	ECombatMode CombatMode = ECombatMode::Ranged;

	bool bCanFire = true;
	bool bCanMelee = true;

	FTimerHandle Timer_FireCooldown;
	FTimerHandle Timer_MeleeCooldown;

	void DoRangedShot();
	void DoMeleeSwing();

	void ResetFire();
	void ResetMelee();

	bool bIsAutoFiring = false;
	FTimerHandle Timer_AutoFire;

	UFUNCTION()
	void AutoFireTick();

	//new 11_18
	// Ranged Energy MVP1 — recharge internals
	FTimerHandle Timer_EnergyRechargeDelay;
	FTimerHandle Timer_EnergyRecharge;

	void StartEnergyRecharge();
	void RechargeEnergyTick();
	void ResetEnergyRechargeDelay();

	bool HasEnoughRangedEnergy(float Cost) const;
	void ConsumeRangedEnergy(float Cost);

	UPROPERTY(EditAnywhere, Category="Combat|Ranged")
	bool bIsFullAuto = true;
};
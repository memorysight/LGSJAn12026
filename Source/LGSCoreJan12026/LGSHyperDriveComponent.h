#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"

#include "LGSHyperDriveComponent.generated.h"

class UNiagaraSystem;
class ACharacter;
class UCharacterMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHyperDriveSimple);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LGSCOREJAN12026_API ULGSHyperDriveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULGSHyperDriveComponent();

	// ===== API =====

	UFUNCTION(BlueprintCallable, Category="HyperDrive")
	void RegisterKill();

	UFUNCTION(BlueprintPure, Category="HyperDrive")
	bool IsHyperDriveActive() const { return bHyperDriveActive; }

	// Called by owning Character's Landed()
	void HandleLanded(const FHitResult& Hit);

	// Railgun TimeShift trigger (called by projectile / hit logic)
	UFUNCTION(BlueprintCallable, Category="HyperDrive|Railgun")
	void NotifyHyperRailgunHit(int32 EnemiesHit, bool bWasKill);

	// FX hooks (bind in BP or character)
	UPROPERTY(BlueprintAssignable, Category="HyperDrive|FX")
	FOnHyperDriveSimple OnHyperDriveStarted;

	UPROPERTY(BlueprintAssignable, Category="HyperDrive|FX")
	FOnHyperDriveSimple OnHyperDriveEnded;

protected:
	virtual void BeginPlay() override;

	// ===== Internals =====
	void BeginHyperDrive();
	void EndHyperDrive();
	void ResetKillStreak();

	// Landing burst
	void TriggerLandingBurst();

	// Railgun TimeShift reset
	void EndBurstTimeShift();

	// Helpers
	ACharacter* GetOwnerCharacter() const;
	UCharacterMovementComponent* GetMoveComp() const;

protected:
	// ===== HyperDrive config =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperDrive|Config")
	float KillStreakWindow = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperDrive|Config")
	int32 KillsForHyperDrive = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperDrive|Config")
	float HyperDriveDuration = 10.f;

	// ===== State =====
	UPROPERTY(BlueprintReadOnly, Category="HyperDrive")
	bool bHyperDriveActive = false;

	UPROPERTY(BlueprintReadOnly, Category="HyperDrive")
	int32 KillStreakCount = 0;

	FTimerHandle Timer_KillStreakWindow;
	FTimerHandle Timer_HyperDriveDuration;

	// ===== Jump boost (MEGA 12/5) =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|HyperDrive")
	bool bHyperDriveJumpBoostEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|HyperDrive", meta=(ClampMin="0.0"))
	float HyperDriveJumpZBoost = 300.f;

	UPROPERTY(BlueprintReadOnly, Category="Movement|HyperDrive")
	float BaseJumpZVelocity = 0.f;

	// ===== HyperRailgun TimeShift (MEGA 11/28) =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperDrive|Railgun|TimeShift")
	bool bHyperRailgunTimeShiftEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperDrive|Railgun|TimeShift", meta=(ClampMin="0.0", ClampMax="1.0"))
	float HyperRailgunTimeShiftChance = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperDrive|Railgun|TimeShift")
	float HyperRailgunGlobalDilation = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperDrive|Railgun|TimeShift")
	float HyperRailgunPlayerDilation = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperDrive|Railgun|TimeShift")
	float HyperRailgunDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperDrive|Railgun|TimeShift")
	int32 HyperRailgunMinEnemies = 1;

	FTimerHandle Timer_BurstTimeShift;

	// Recommended: restore previous values (safer than forcing 1.0)
	float CachedPrevGlobalDilation = 1.0f;
	float CachedPrevOwnerDilation  = 1.0f;

	// ===== Landing Burst (MEGA 12/10) =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LandingBurst")
	bool bLandingBurstEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LandingBurst", meta=(ClampMin="0.0"))
	float LandingBurstMinVelocity = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LandingBurst")
	float LandingBurstRadius = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LandingBurst")
	float LandingBurstBaseDamage = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LandingBurst")
	float LandingBurstGodDamageMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LandingBurst")
	float LandingBurstDudDamageMultiplier = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LandingBurst")
	float LandingBurstKnockbackStrength = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LandingBurst|FX")
	UNiagaraSystem* LandingBurstAverageFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LandingBurst|FX")
	UNiagaraSystem* LandingBurstGodFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LandingBurst|FX")
	UNiagaraSystem* LandingBurstDudFX = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="LandingBurst")
	bool bHasUsedLandingBurstThisHyperDrive = false;
};
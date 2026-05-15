#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
//new 5_15 EventHorizon
class UNiagaraSystem;
class UNiagaraComponent;
//end 5_15
#include "LGSOverDriveComponent.generated.h"

class ACharacter;


//new 5_15 EventHorizon
// Normal melee = physical.
// OverDrive melee = the anatomy of time starts breaking.

UENUM(BlueprintType)
enum class EEventHorizonTier : uint8
{
	Normal,
	Dud,
	Average,
	God
};
//end 5_15

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class LGSCOREJAN12026_API ULGSOverDriveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULGSOverDriveComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "Combat|OverDrive")
	float GetOverDriveCharge() const { return OverDriveCharge; }

	UFUNCTION(BlueprintPure, Category = "Combat|OverDrive")
	float GetOverDriveMaxCharge() const { return OverDriveMaxCharge; }

	UFUNCTION(BlueprintPure, Category = "Combat|OverDrive")
	float GetOverDrivePercent() const;

	UFUNCTION(BlueprintPure, Category = "Combat|OverDrive")
	bool IsOverDriveReady() const { return OverDriveCharge >= OverDriveThreshold; }

	UFUNCTION(BlueprintPure, Category = "Combat|OverDrive")
	bool IsOverDriveActive() const { return bOverDriveActive; }

	UFUNCTION(BlueprintCallable, Category = "Combat|OverDrive")
	void AddOverDriveCharge(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Combat|OverDrive")
	void ResetOverDrive();

	UFUNCTION(BlueprintCallable, Category = "Combat|OverDrive")
	void ActivateOverDrive();

	UFUNCTION(BlueprintCallable, Category = "Combat|OverDrive")
	void DeactivateOverDrive();

	//5_13 OverDriveMeterKillStreak
	UFUNCTION(BlueprintCallable, Category = "Combat|OverDrive|Kill Streak")
	void RegisterMeleeKill();

	UFUNCTION(BlueprintCallable, Category = "Combat|OverDrive|Kill Streak")
	void ResetMeleeKillStreak();

	UFUNCTION(BlueprintPure, Category = "Combat|OverDrive|Kill Streak")
	int32 GetMeleeKillStreakCount() const { return MeleeKillStreakCount; }
	//end 5_13

	//new 5_15 EventHorizon
	// Event Horizon / SmartMove
	UPROPERTY(BlueprintReadOnly, Category="OverDrive|EventHorizon")
	bool bChargingEventHorizon = false;

	UPROPERTY(BlueprintReadOnly, Category="OverDrive|EventHorizon")
	AActor* LockedEventHorizonTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OverDrive|EventHorizon")
	float EventHorizonTargetSearchRadius = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OverDrive|EventHorizon")
	float EventHorizonTargetMaxAngleDegrees = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OverDrive|EventHorizon")
	float EventHorizonChargeMoveSpeedMultiplier = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OverDrive|EventHorizon")
	float EventHorizonMinChargeTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OverDrive|EventHorizon")
	float EventHorizonMaxChargeTime = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OverDrive|EventHorizon")
	float GodChance = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OverDrive|EventHorizon")
	float AverageChance = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OverDrive|Melee")
	float ODEnemyRuptureChance = 0.20f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OverDrive|FX")
	TArray<UNiagaraSystem*> ODMeleeSwingFXPool;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OverDrive|FX")
	UNiagaraSystem* EventHorizonNormalFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OverDrive|FX")
	UNiagaraSystem* EventHorizonDudFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OverDrive|FX")
	UNiagaraSystem* EventHorizonAverageFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OverDrive|FX")
	UNiagaraSystem* EventHorizonGodFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OverDrive|FX")
	UNiagaraSystem* ODRuptureFX = nullptr;

	//5_15 important additional declarations
	UFUNCTION(BlueprintCallable, Category="OverDrive|EventHorizon")
	void StartEventHorizonCharge();

	UFUNCTION(BlueprintCallable, Category="OverDrive|EventHorizon")
	void ReleaseEventHorizon();

	//new 5_15 so clickable is not EH, but regular swing
	UFUNCTION(BlueprintPure, Category="OverDrive|EventHorizon")
	float GetEventHorizonHeldTime() const;

	UFUNCTION(BlueprintCallable, Category="OverDrive|EventHorizon")
	void CancelEventHorizonCharge();

	//5_15 very important
	void SpawnEventHorizonFX(UNiagaraSystem* FX, AActor* Target);
	void ApplyEventHorizonDamage(AActor* Target, float Damage, float Knockback);

	UFUNCTION(BlueprintCallable, Category="OverDrive|EventHorizon")
	AActor* FindBestEventHorizonTarget();

	void ExecuteEventHorizon(EEventHorizonTier Tier, AActor* Target, float HeldTime);
	void SpawnRandomODSwingTrail();
	void TryODRupture(AActor* Target);
	void SpawnEventHorizonLockFX(AActor* Target);
	void ClearEventHorizonTarget();
	void DebugOD(const FString& Message, const FColor& Color = FColor::White, float Time = 1.5f);

	float EventHorizonChargeStartTime = 0.f;
	float CachedWalkSpeedBeforeEH = 600.f;
	//end 5_15

	//new 5_15 expose Niagara Slot
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OverDrive|EventHorizon|FX")
	UNiagaraSystem* EventHorizonLockOnFX = nullptr;

	UPROPERTY()
	UNiagaraComponent* ActiveEventHorizonLockFX = nullptr;
	//end 5_15


	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Combat|OverDrive")
	ACharacter* OwnerCharacter = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|OverDrive")
	float OverDriveMaxCharge = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|OverDrive")
	float OverDriveThreshold = 100.f;

	// For the first pass, 1 damage = 1 OD charge.
	// Later we can tune this per weapon, target type, air state, combo state, etc.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|OverDrive")
	float DamageToChargeMultiplier = 1.f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat|OverDrive")
	float OverDriveCharge = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat|OverDrive")
	bool bOverDriveActive = false;

	//new 5_13 OverDrive MeterKillStreak
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|OverDrive|Kill Streak")
	int32 KillsForOverDrive = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|OverDrive|Kill Streak")
	float MeleeKillStreakWindow = 6.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat|OverDrive|Kill Streak")
	int32 MeleeKillStreakCount = 0;

	FTimerHandle Timer_MeleeKillStreakWindow;
};
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LGSShieldComponent.generated.h"

UCLASS(ClassGroup=(LGS), meta=(BlueprintSpawnableComponent))
class LGSCOREJAN12026_API ULGSShieldComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULGSShieldComponent();

	// Toggle/State
	UFUNCTION(BlueprintCallable, Category="Shield")
	void ToggleShield();

	UFUNCTION(BlueprintCallable, Category="Shield")
	void ActivateShield();

	UFUNCTION(BlueprintCallable, Category="Shield")
	void DeactivateShield();

	UFUNCTION(BlueprintPure, Category="Shield")
	bool IsShieldActiveAndPowered() const { return bShieldActive && !bShieldBroken && ShieldEnergy > 0.f; }

	// Damage gate: returns REMAINING damage after shield absorbs
	float HandleIncomingDamage(float DamageAmount);

	UFUNCTION(BlueprintPure, Category="Shield")
	float GetShieldEnergy() const { return ShieldEnergy; }

	UFUNCTION(BlueprintPure, Category="Shield")
	float GetMaxShieldEnergy() const { return MaxShieldEnergy; }

protected:
	virtual void BeginPlay() override;

private:
	void StartDrain();
	void StopDrain();
	void DrainTick();

	void StartRegenDelay();
	void StartRegen();
	void StopRegen();
	void RegenTick();

	void BreakShield();
	void UpdateEligibilityAndVisuals();

private:
	UPROPERTY(EditAnywhere, Category="Shield|Config")
	float MaxShieldEnergy = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shield|Runtime", meta=(AllowPrivateAccess="true"))
	float ShieldEnergy = 100.f;

	UPROPERTY(EditAnywhere, Category="Shield|Config")
	float DrainPerSecond = 20.f;

	UPROPERTY(EditAnywhere, Category="Shield|Config")
	float RegenPerSecond = 10.f;

	UPROPERTY(EditAnywhere, Category="Shield|Config")
	float RegenDelaySeconds = 1.5f;

	UPROPERTY(BlueprintReadOnly, Category="Shield|State", meta=(AllowPrivateAccess="true"))
	bool bShieldActive = false;

	UPROPERTY(BlueprintReadOnly, Category="Shield|State", meta=(AllowPrivateAccess="true"))
	bool bShieldBroken = false;

	FTimerHandle Timer_Drain;
	FTimerHandle Timer_Regen;
	FTimerHandle Timer_RegenDelay;
};

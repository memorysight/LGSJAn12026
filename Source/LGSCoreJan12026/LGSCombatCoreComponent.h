#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
//new 1_3_26
#include "Templates/SubclassOf.h"
#include "TimerManager.h"
//end 1_3_26
#include "LGSCombatCoreComponent.generated.h"

//new 1_3_26
class UAnimMontage;
class AActor; // optional (TSubclassOf generally fine, but harmless)
//end 1_3_26

UENUM(BlueprintType)
enum class ECombatMode : uint8
{
	Ranged UMETA(DisplayName="Ranged"),
	Melee  UMETA(DisplayName="Melee")
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatModeChanged, ECombatMode, NewMode);

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
	void TryPrimary();   // LMB

	UFUNCTION(BlueprintCallable, Category="Combat|Input")
	void TrySecondary(); // RMB (reserved)

	// --- Ranged config ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Ranged")
	TSubclassOf<AActor> BulletClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Ranged")
	FName MuzzleSocketName = TEXT("Muzzle");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Ranged")
	float FireCooldown = 0.10f;

	// --- Melee config ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Melee")
	UAnimMontage* MeleeMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Melee")
	float MeleeCooldown = 0.35f;

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
};

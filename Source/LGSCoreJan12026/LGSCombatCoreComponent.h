#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
//1_3_26
#include "Templates/SubclassOf.h"
#include "TimerManager.h"
//end 1_3_26
#include "LGSCombatCoreComponent.generated.h"

//1_3_26
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

	//2_1 BeginMeleeDamage
	UFUNCTION(BlueprintCallable, Category="Combat|Melee")
	void BeginMeleeDamage();
	//end 2_1

	//new 2_2 EndMeleeDamage
	UFUNCTION(BlueprintCallable, Category="Combat|Melee")
	void EndMeleeDamage();
	//end 2_2



protected:
	virtual void BeginPlay() override;

private:
	//might be set to melee in the editor and throwing off assignment when toggling
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

	//new 2_2 TraceWindow
	// Damage window state
	bool bMeleeDamageActive = false;

	// Prevent hitting same actor multiple times in one swing
	TSet<TWeakObjectPtr<AActor>> HitActorsThisSwing;

	// Melee trace tuning
	UPROPERTY(EditAnywhere, Category="Combat|Melee|Trace")
	float MeleeTraceDistance = 200.f;

	UPROPERTY(EditAnywhere, Category="Combat|Melee|Trace")
	float MeleeTraceRadius = 35.f;

	UPROPERTY(EditAnywhere, Category="Combat|Melee|Trace")
	float MeleeDamage = 25.f;

	// UPROPERTY(EditAnywhere, Category="Combat|Melee|Trace")
	// TEnumAsByte<ECollisionChannel> MeleeTraceChannel = ECC_Pawn;
	
	UPROPERTY(EditAnywhere, Category="Combat|Melee|Trace")
	TEnumAsByte<ECollisionChannel> MeleeTraceChannel = ECC_Visibility;


	// UPROPERTY(EditAnywhere, Category="Combat|Melee|Trace")
	// bool bDrawMeleeDebug = false;

	UPROPERTY(EditAnywhere, Category="Combat|Melee|Trace")
	bool bDrawMeleeDebug = true;

	UPROPERTY(EditAnywhere, Category="Combat|Melee|Trace")
	FName MeleeTraceSocketName = TEXT("WeaponSocket_R"); // or "hand_r"

	FTimerHandle Timer_MeleeTrace;

	void PerformMeleeTrace();
	void StartMeleeTraceLoop();
	void StopMeleeTraceLoop();
	//end 2_2
};

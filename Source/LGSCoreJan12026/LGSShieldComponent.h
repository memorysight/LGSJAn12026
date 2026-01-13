#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
//new 1_14_26
#include "Engine/EngineTypes.h"
#include "Templates/SubclassOf.h"
#include "Components/PrimitiveComponent.h"
//end 1_14_26
#include "LGSShieldComponent.generated.h"

//new 1_14_26
class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;
class UMaterialInterface;
//end 1_14_26


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

	//new 1_14_26
	UFUNCTION()
	void OnShieldBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	// Optional: let you tune/assign in BP
	UPROPERTY(EditAnywhere, Category="Shield|Collision")
	bool bDestroyBulletsOnOverlap = true;

	UPROPERTY(EditAnywhere, Category="Shield|Collision")
	TSubclassOf<AActor> BulletClass;

	UPROPERTY(EditAnywhere, Category="Shield|Collision")
	float BulletImpactCost = 1.f;

	UPROPERTY(EditAnywhere, Category="Shield|Collision")
	TEnumAsByte<ECollisionChannel> ProjectileChannel = ECC_WorldDynamic;


	// Components (created at runtime)
	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> ShieldRootComp = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USphereComponent> ShieldCollisionComp = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> ShieldVisualComp = nullptr;

	// Visuals
	UPROPERTY(EditAnywhere, Category="Shield|Visual")
	UMaterialInterface* ShieldMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category="Shield|Visual")
	float ShieldSphereRadius = 65.f;
	//end 1_14_26

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

	//new _1_14_26
	AActor* GetOwnerActorChecked() const;
	//end 1_14_26

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

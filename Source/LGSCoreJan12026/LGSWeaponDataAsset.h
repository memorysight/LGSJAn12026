#pragma once


#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NiagaraSystem.h"
#include "Engine/StaticMesh.h"
#include "LGSWeaponDataAsset.generated.h"


class UStaticMesh;
class UAnimMontage;
class UNiagaraSystem;
class USoundBase;
class AActor;

/**
 * Data-driven weapon definition (ranged or melee).
 * Keeps Blueprints light, supports RNG tiers, future TP visuals, etc.
 */
UENUM(BlueprintType)
enum class ELGSWeaponType : uint8
{
	Ranged UMETA(DisplayName="Ranged"),
	Melee  UMETA(DisplayName="Melee")
};

UCLASS(BlueprintType)
class LGSCOREJAN12026_API ULGSWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- Identity ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	FName WeaponId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	ELGSWeaponType WeaponType = ELGSWeaponType::Ranged;

	// --- Visuals (First person / Third person) ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Visuals")
	UStaticMesh* FP_StaticMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Visuals")
	UStaticMesh* TP_StaticMesh = nullptr; // optional for later

	/** Optional additional offset after socket snap (good for alignment tweaks). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Visuals")
	FTransform AttachOffset = FTransform::Identity;

	/** If set, overrides the default socket name for this weapon (rare). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Visuals")
	FName AttachSocketOverride = NAME_None;

	// --- Ranged config ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Ranged", meta=(EditCondition="WeaponType==ELGSWeaponType::Ranged"))
	TSubclassOf<AActor> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Ranged", meta=(EditCondition="WeaponType==ELGSWeaponType::Ranged"))
	FName MuzzleSocketName = TEXT("weapon_r_muzzle");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Ranged", meta=(EditCondition="WeaponType==ELGSWeaponType::Ranged", ClampMin="0.01"))
	float FireCooldown = 0.10f;

	// --- Melee config ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Melee", meta=(EditCondition="WeaponType==ELGSWeaponType::Melee"))
	UAnimMontage* MeleeMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Melee", meta=(EditCondition="WeaponType==ELGSWeaponType::Melee", ClampMin="0.01"))
	float MeleeCooldown = 0.35f;

	// --- Optional feedback (later hooks) ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|FX")
	UNiagaraSystem* FireFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|FX")
	USoundBase* FireSFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Visual")
	TObjectPtr<UStaticMesh> WeaponMesh = nullptr;

};

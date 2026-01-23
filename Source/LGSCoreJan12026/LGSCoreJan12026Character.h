// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
//1_3_26
#include "LGSCombatCoreComponent.h"
//end 1_3_26
//1/4/26
#include "LGSWeaponDataAsset.h"
//end 1_4_26
//new 1_23_26
class UInputAction;
//end 1_23_26
#include "LGSCoreJan12026Character.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputMappingContext;
class ULGSCombatCoreComponent;
//1_19_26
class ULGSShieldComponent;
class USphereComponent;
class UStaticMeshComponent;
class USceneComponent;
//end_1_19


struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ALGSCoreJan12026Character : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	//new 1_23_26
	// Shield input
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInputAction> ToggleShieldAction;

	UFUNCTION()
	void OnToggleShieldPressed();
	
	//end 1_23_26
	


	//1/2/26
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat", meta=(AllowPrivateAccess="true"))
	ULGSCombatCoreComponent* CombatCore;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* ShootAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* ToggleCombatModeAction;
	//end 1/2/26

	//1_3_26
	// --- Weapon visuals (mode swap) ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapons", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* RangedWeaponVisual = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapons", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* MeleeWeaponVisual = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapons|Sockets", meta=(AllowPrivateAccess="true"))
	FName RangedWeaponSocketName = TEXT("weapon_r");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapons|Sockets", meta=(AllowPrivateAccess="true"))
	FName MeleeWeaponSocketName = TEXT("weapon_r");

	//1_6_26
	UPROPERTY(EditDefaultsOnly, Category="Weapons|Fixups")
	FRotator RangedVisualRotationFix = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, Category="Weapons|Fixups")
	FRotator MeleeVisualRotationFix = FRotator::ZeroRotator;

	//1_19_26
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shield", meta=(AllowPrivateAccess="true"))
	ULGSShieldComponent* ShieldComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shield|Components", meta=(AllowPrivateAccess="true"))
	USceneComponent* ShieldRootComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shield|Components", meta=(AllowPrivateAccess="true"))
	USphereComponent* ShieldCollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shield|Components", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* ShieldVisualComp;


	//end 1_19

	// DataAssets
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapons|Data", meta=(AllowPrivateAccess="true"))
	ULGSWeaponDataAsset* RangedWeaponData = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapons|Data", meta=(AllowPrivateAccess="true"))
	ULGSWeaponDataAsset* MeleeWeaponData = nullptr;

	UFUNCTION()
	void HandleCombatModeChanged(ECombatMode NewMode);

	UFUNCTION()
	void ApplyWeaponVisualsForMode(ECombatMode NewMode);

	
	
public:
	ALGSCoreJan12026Character();

	
	USceneComponent* GetShieldRootComp() const { return ShieldRootComp; }
	USphereComponent* GetShieldCollisionComp() const { return ShieldCollisionComp; }
	UStaticMeshComponent* GetShieldVisualComp() const { return ShieldVisualComp; }


protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	//1_3_26
	virtual void BeginPlay() override;
	//end 1_3_26

protected:
	// APawn interface
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};


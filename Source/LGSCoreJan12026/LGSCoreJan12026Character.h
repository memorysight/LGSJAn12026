// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "InputAction.h"
//new 1_3_26
#include "LGSCombatCoreComponent.h"
//end 1_3_26
#include "LGSCoreJan12026Character.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class ULGSCombatCoreComponent;
//new 1_3_26
class UStaticMeshComponent;
//end 1_3_26
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

	//1/2/26
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat", meta=(AllowPrivateAccess="true"))
	ULGSCombatCoreComponent* CombatCore;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* ShootAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* ToggleCombatModeAction;
	//end 1/2/26

	//new 1_3_26
	// --- Weapon visuals (mode swap) ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapons", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* RangedWeaponVisual = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapons", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* MeleeWeaponVisual = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapons|Sockets", meta=(AllowPrivateAccess="true"))
	FName RangedWeaponSocketName = TEXT("WeaponSocket");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapons|Sockets", meta=(AllowPrivateAccess="true"))
	FName MeleeWeaponSocketName = TEXT("hand_rSocket");

	UFUNCTION()
	void HandleCombatModeChanged(ECombatMode NewMode);

	void ApplyWeaponVisualsForMode(ECombatMode NewMode);
	//end 1_3_26

	
public:
	ALGSCoreJan12026Character();

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	//new 1_3_26
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


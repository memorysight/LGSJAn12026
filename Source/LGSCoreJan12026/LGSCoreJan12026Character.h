
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
//1_3_26
#include "LGSCombatCoreComponent.h"
//end 1_3_26
//new 7_14_Omega
class USpringArmComponent;
//end 7_14
//new 8_14 MVS
class ULGSMVSComponent;
//end 8_14 MVS
//1/4/26
#include "LGSWeaponDataAsset.h"
#include "InputMappingContext.h"
//end 1_4_26
//1_23_26
class UInputAction;
//end 1_23_26
//1_27
class ULGSAirWalkComponent;
class ULGSHyperDriveComponent;
//end 1_27
//4_29 OverDrive
class ULGSOverDriveComponent;
//4_29 


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

#include "LGSCoreJan12026Character.generated.h"
//end_1_19




struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config = Game)
class ALGSCoreJan12026Character : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */ 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	//1_23_26
	// Shield input
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ToggleShieldAction;

	UFUNCTION()
	void OnToggleShieldPressed();
	//end 1_23_26

	//HyperDrive 1_27_26
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HyperDrive", meta = (AllowPrivateAccess = "true"))
	ULGSHyperDriveComponent* HyperDriveComp = nullptr;

	virtual void Landed(const FHitResult& Hit) override;
	//end 1_27_26

	//new 4_29 OverDrive
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OverDrive", meta = (AllowPrivateAccess = "true"))
	ULGSOverDriveComponent* OverDriveComp = nullptr;
	//end 4_29

	//new 7_14 OmegaDrive Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OmegaDrive|Camera",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> OmegaSpringArm = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OmegaDrive|Camera",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> OmegaThirdPersonCamera = nullptr;
	//end 7_14

	//new 7_28 Omega Serath Runtime Mesh

	UPROPERTY(VisibleAnywhere,
		BlueprintReadOnly,
		Category="OmegaDrive|Mesh",
		meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> OmegaSerathMesh = nullptr;

	//end 7_28

	//new 8_14 MVS First Music in LGS
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MVS",
	meta=(AllowPrivateAccess="true"))
	TObjectPtr<ULGSMVSComponent> MVSComponent;
	//end 8_14
	
	

	//new 7_14 OmegaDrive Input
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> OmegaDriveAction = nullptr;
	//end 7_14

	//3_12_Sprint   
	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;
	//end 3_12_Sprint

	// 3_13_Crouch
	/** Crouch Input Action */

	UFUNCTION()
	void OnCrouchStarted();

	UFUNCTION()
	void OnCrouchReleased();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;
	// end 3_13_Crouch

	//1/2/26
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	ULGSCombatCoreComponent* CombatCore;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ShootAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ToggleCombatModeAction;
	//end 1/2/26

	//1_3_26
	// --- Weapon visuals (mode swap) ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapons", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* RangedWeaponVisual = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapons", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* MeleeWeaponVisual = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapons|Sockets", meta = (AllowPrivateAccess = "true"))
	FName RangedWeaponSocketName = TEXT("weapon_r");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapons|Sockets", meta = (AllowPrivateAccess = "true"))
	FName MeleeWeaponSocketName = TEXT("weapon_r");

	//1_6_26
	UPROPERTY(EditDefaultsOnly, Category = "Weapons|Fixups")
	FRotator RangedVisualRotationFix = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, Category = "Weapons|Fixups")
	FRotator MeleeVisualRotationFix = FRotator::ZeroRotator;

	//combat mode 1_30
	//seems to be already existing in code see 152 & 155 
	// UFUNCTION()     
	// void HandleCombatModeChanged(ECombatMode NewMode);
	//
	// void ApplyWeaponVisualsForMode(ECombatMode NewMode);

	static const FName WeaponSocketName;
	//end 1_30

	//1_19_26
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shield", meta = (AllowPrivateAccess = "true"))
	ULGSShieldComponent* ShieldComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shield|Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* ShieldRootComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shield|Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* ShieldCollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shield|Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ShieldVisualComp;
	//end 1_19

	//3_27 AirWalk
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AirWalkAction = nullptr;

	UFUNCTION()
	void OnAirWalkStarted();

	UFUNCTION()
	void OnAirWalkReleased();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|AirWalk", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULGSAirWalkComponent> AirWalkComp = nullptr;

	//5_15 EventHorizon ChargeEnablement
	void OnPrimaryStarted();
	void OnPrimaryReleased();
	//end 5_15

	// DataAssets
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapons|Data", meta = (AllowPrivateAccess = "true"))
	ULGSWeaponDataAsset* RangedWeaponData = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapons|Data", meta = (AllowPrivateAccess = "true"))
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

	//2_23
	UStaticMeshComponent* GetMeleeWeaponVisual() const { return MeleeWeaponVisual; }
	//end 2_23

	//4_14_AirWalk
	// Small helper so AirWalk component does not touch sprint internals
	UFUNCTION(BlueprintCallable, Category = "Movement|Sprint")
	void CancelSprintForAirWalk();

	// Optional convenience passthrough
	UFUNCTION(BlueprintPure, Category = "Movement|AirWalk")
	bool IsAirWalkActive() const;

	//4_15 Airwalk Updated
	UFUNCTION(BlueprintPure, Category = "Movement")
	FVector2D GetLastMoveInput() const { return LastMoveInput; }

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	FVector2D LastMoveInput = FVector2D::ZeroVector;
	//end 4_14

	//new 7_14 OmegaDrive First Pass__ClothC++7_28Update__MVSLab8_13Update2__WebSocketsDTO9_3Update3 
	UFUNCTION()
	void OnOmegaDrivePressed();

	UFUNCTION(BlueprintCallable, Category = "Combat|OmegaDrive")
	void ActivateOmegaDrive();

	UFUNCTION(BlueprintCallable, Category = "Combat|OmegaDrive")
	void DeactivateOmegaDrive();

	UFUNCTION(BlueprintPure, Category = "Combat|OmegaDrive")
	bool IsOmegaDriveReady() const;

	UFUNCTION(BlueprintPure, Category = "Combat|OmegaDrive")
	bool IsOmegaDriveActive() const { return bOmegaDriveActive; }
	//end 7_14

protected:
	/** Called for movement input */ 
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	//1_3_26
	virtual void BeginPlay() override;
	//end 1_3_26

	//3_12_Sprint
	UFUNCTION()
	void OnSprintStarted();

	UFUNCTION()
	void OnSprintReleased();

	void UpdateSprintState();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Sprint", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Sprint", meta = (AllowPrivateAccess = "true"))
	float SprintSpeed = 950.f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Sprint", meta = (AllowPrivateAccess = "true"))
	bool bSprintHeld = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Sprint", meta = (AllowPrivateAccess = "true"))
	bool bIsSprinting = false;
	//end 3_12_Sprint

	// //3_27
	// virtual void Tick(float DeltaSeconds) override;
	// //end 3_27

	//new 7_14 OmegaDrive State
	UPROPERTY(BlueprintReadOnly, Category = "Combat|OmegaDrive",
		meta = (AllowPrivateAccess = "true"))
	bool bOmegaDriveActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|OmegaDrive")
	float OmegaDriveDuration = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|OmegaDrive|Movement")
	float OmegaWalkSpeed = 1100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|OmegaDrive|Movement")
	float OmegaJumpZVelocity = 900.f;

	FTimerHandle Timer_OmegaDriveDuration;

	float PreOmegaWalkSpeed = 0.f;
	float PreOmegaJumpZVelocity = 0.f;

	bool bPreOmegaUseControllerRotationYaw = true;
	bool bPreOmegaOrientRotationToMovement = false;
	//end 7_14



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


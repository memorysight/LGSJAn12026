#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LGSOverDriveComponent.generated.h"

class ACharacter;

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

	//new 5_13 OverDriveMeterKillStreak
	UFUNCTION(BlueprintCallable, Category = "Combat|OverDrive|Kill Streak")
	void RegisterMeleeKill();

	UFUNCTION(BlueprintCallable, Category = "Combat|OverDrive|Kill Streak")
	void ResetMeleeKillStreak();

	UFUNCTION(BlueprintPure, Category = "Combat|OverDrive|Kill Streak")
	int32 GetMeleeKillStreakCount() const { return MeleeKillStreakCount; }
	//end 5_13

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
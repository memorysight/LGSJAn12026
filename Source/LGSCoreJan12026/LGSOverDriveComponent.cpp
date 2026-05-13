#include "LGSOverDriveComponent.h"

#include "GameFramework/Character.h"
//new 5_13 ODKillStreak
#include "TimerManager.h"
#include "Engine/World.h"
//end 5_13
#include "Engine/Engine.h"

ULGSOverDriveComponent::ULGSOverDriveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULGSOverDriveComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());

	UE_LOG(LogTemp, Warning, TEXT("[OD] OverDrive Component BeginPlay Owner=%s"),
		*GetNameSafe(OwnerCharacter));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.5f,
			FColor::Orange,
			FString::Printf(TEXT("[OD] OverDrive Online: %s"), *GetNameSafe(OwnerCharacter))
		);
	}
}

//new 5_13 OverDrive KillStreak
void ULGSOverDriveComponent::RegisterMeleeKill()
{
	if (!GetWorld())
	{
		return;
	}

	if (bOverDriveActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OD] RegisterMeleeKill ignored - OverDrive already active"));
		return;
	}

	MeleeKillStreakCount++;

	GetWorld()->GetTimerManager().ClearTimer(Timer_MeleeKillStreakWindow);
	GetWorld()->GetTimerManager().SetTimer(
		Timer_MeleeKillStreakWindow,
		this,
		&ULGSOverDriveComponent::ResetMeleeKillStreak,
		MeleeKillStreakWindow,
		false
	);

	UE_LOG(LogTemp, Warning, TEXT("[OD] Melee Kill Registered Count=%d / %d"),
		MeleeKillStreakCount,
		KillsForOverDrive);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.5f,
			FColor::Orange,
			FString::Printf(TEXT("[OD] Melee Kill %d / %d"), MeleeKillStreakCount, KillsForOverDrive)
		);
	}

	if (MeleeKillStreakCount >= KillsForOverDrive)
	{
		OverDriveCharge = OverDriveMaxCharge;
		ActivateOverDrive();

		GetWorld()->GetTimerManager().ClearTimer(Timer_MeleeKillStreakWindow);
		MeleeKillStreakCount = 0;
	}
}

void ULGSOverDriveComponent::ResetMeleeKillStreak()
{
	if (bOverDriveActive)
	{
		return;
	}

	MeleeKillStreakCount = 0;

	UE_LOG(LogTemp, Warning, TEXT("[OD] Melee Kill Streak Reset"));
}



float ULGSOverDriveComponent::GetOverDrivePercent() const
{
	if (OverDriveMaxCharge <= 0.f)
	{
		return 0.f;
	}

	return FMath::Clamp(OverDriveCharge / OverDriveMaxCharge, 0.f, 1.f);
}

void ULGSOverDriveComponent::AddOverDriveCharge(float DamageAmount)
{
	if (DamageAmount <= 0.f || OverDriveMaxCharge <= 0.f)
	{
		return;
	}

	const float ChargeToAdd = DamageAmount * DamageToChargeMultiplier;

	OverDriveCharge = FMath::Clamp(
		OverDriveCharge + ChargeToAdd,
		0.f,
		OverDriveMaxCharge
	);

	UE_LOG(LogTemp, Warning, TEXT("[OD] Charge Added Damage=%.1f Charge=%.1f / %.1f Percent=%.2f Ready=%d"),
		DamageAmount,
		OverDriveCharge,
		OverDriveMaxCharge,
		GetOverDrivePercent(),
		IsOverDriveReady() ? 1 : 0);
}

void ULGSOverDriveComponent::ResetOverDrive()
{
	OverDriveCharge = 0.f;
	bOverDriveActive = false;

	UE_LOG(LogTemp, Warning, TEXT("[OD] Reset"));

	//new 5_13 ODKillingStreak
	MeleeKillStreakCount = 0;

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(Timer_MeleeKillStreakWindow);
	}
}

void ULGSOverDriveComponent::ActivateOverDrive()
{
	if (!IsOverDriveReady())
	{
		UE_LOG(LogTemp, Warning, TEXT("[OD] Activate blocked - not ready Charge=%.1f / %.1f"),
			OverDriveCharge,
			OverDriveThreshold);
		return;
	}

	bOverDriveActive = true;

	UE_LOG(LogTemp, Warning, TEXT("[OD] ACTIVATED"));
}

void ULGSOverDriveComponent::DeactivateOverDrive()
{
	bOverDriveActive = false;

	UE_LOG(LogTemp, Warning, TEXT("[OD] Deactivated"));

	//new 5_13 ODKillingStreak Cleanup
	MeleeKillStreakCount = 0;

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(Timer_MeleeKillStreakWindow);
	}
}
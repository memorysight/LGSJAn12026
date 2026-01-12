#include "LGSShieldComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

ULGSShieldComponent::ULGSShieldComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULGSShieldComponent::BeginPlay()
{
	Super::BeginPlay();

	ShieldEnergy = FMath::Clamp(ShieldEnergy, 0.f, MaxShieldEnergy);
	UpdateEligibilityAndVisuals();
}

void ULGSShieldComponent::ToggleShield()
{
	if (bShieldActive) DeactivateShield();
	else ActivateShield();
}

void ULGSShieldComponent::ActivateShield()
{
	if (bShieldBroken || ShieldEnergy <= 0.f) return;

	bShieldActive = true;
	StopRegen();
	StartDrain();
	UpdateEligibilityAndVisuals();
}

void ULGSShieldComponent::DeactivateShield()
{
	if (!bShieldActive) return;

	bShieldActive = false;
	StopDrain();
	StartRegenDelay();
	UpdateEligibilityAndVisuals();
}

float ULGSShieldComponent::HandleIncomingDamage(float DamageAmount)
{
	if (!IsShieldActiveAndPowered() || DamageAmount <= 0.f)
	{
		return DamageAmount; // all damage passes through
	}

	const float Absorb = FMath::Min(ShieldEnergy, DamageAmount);
	ShieldEnergy -= Absorb;

	if (ShieldEnergy <= 0.f)
	{
		BreakShield();
		return DamageAmount - Absorb; // leftover
	}

	// If we took damage, kick regen timer
	StartRegenDelay();

	UpdateEligibilityAndVisuals();
	return DamageAmount - Absorb;
}

void ULGSShieldComponent::StartDrain()
{
	UWorld* W = GetWorld();
	if (!W) return;

	if (!W->GetTimerManager().IsTimerActive(Timer_Drain))
	{
		W->GetTimerManager().SetTimer(Timer_Drain, this, &ULGSShieldComponent::DrainTick, 0.1f, true);
	}
}

void ULGSShieldComponent::StopDrain()
{
	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(Timer_Drain);
	}
}

void ULGSShieldComponent::DrainTick()
{
	if (!bShieldActive) return;

	const float Step = DrainPerSecond * 0.1f;
	ShieldEnergy = FMath::Max(0.f, ShieldEnergy - Step);

	if (ShieldEnergy <= 0.f)
	{
		BreakShield();
	}
	UpdateEligibilityAndVisuals();
}

void ULGSShieldComponent::StartRegenDelay()
{
	UWorld* W = GetWorld();
	if (!W) return;

	StopRegen();

	W->GetTimerManager().ClearTimer(Timer_RegenDelay);
	W->GetTimerManager().SetTimer(
		Timer_RegenDelay,
		this,
		&ULGSShieldComponent::StartRegen,
		RegenDelaySeconds,
		false
	);
}

void ULGSShieldComponent::StartRegen()
{
	UWorld* W = GetWorld();
	if (!W) return;
	if (bShieldActive) return;

	if (!W->GetTimerManager().IsTimerActive(Timer_Regen))
	{
		W->GetTimerManager().SetTimer(Timer_Regen, this, &ULGSShieldComponent::RegenTick, 0.1f, true);
	}
}

void ULGSShieldComponent::StopRegen()
{
	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(Timer_Regen);
		W->GetTimerManager().ClearTimer(Timer_RegenDelay);
	}
}

void ULGSShieldComponent::RegenTick()
{
	if (bShieldActive)
	{
		StopRegen();
		return;
	}

	const float Step = RegenPerSecond * 0.1f;
	ShieldEnergy = FMath::Min(MaxShieldEnergy, ShieldEnergy + Step);

	if (bShieldBroken && ShieldEnergy >= FMath::Max(5.f, 0.05f * MaxShieldEnergy))
	{
		bShieldBroken = false;
	}

	if (ShieldEnergy >= MaxShieldEnergy)
	{
		StopRegen();
	}

	UpdateEligibilityAndVisuals();
}

void ULGSShieldComponent::BreakShield()
{
	ShieldEnergy = 0.f;
	bShieldBroken = true;
	bShieldActive = false;

	StopDrain();
	StartRegenDelay();
	UpdateEligibilityAndVisuals();
}

void ULGSShieldComponent::UpdateEligibilityAndVisuals()
{
	// Phase 1: no visuals. Keep this function for later.
}

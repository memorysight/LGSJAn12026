#include "LGSShieldComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
//new 1_14_26
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
//end 1_14_26

ULGSShieldComponent::ULGSShieldComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//new 1_14_26
static UStaticMesh* GetEngineSphereMesh()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshObj(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	return SphereMeshObj.Succeeded() ? SphereMeshObj.Object : nullptr;
}
//end 1_14_26

void ULGSShieldComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner) return;

	// Choose best attach parent: Character mesh if possible, else root.
	USceneComponent* AttachParent = Owner->GetRootComponent();

	if (ACharacter* C = Cast<ACharacter>(Owner))
	{
		if (USkeletalMeshComponent* Mesh = C->GetMesh())
		{
			AttachParent = Mesh;
		}
	}

	if (!AttachParent) return;

	// Create shield parts once
	if (!ShieldRootComp)
	{
		ShieldRootComp = NewObject<USceneComponent>(Owner, TEXT("ShieldRootComp"));
		ShieldRootComp->RegisterComponent();
		ShieldRootComp->AttachToComponent(AttachParent, FAttachmentTransformRules::KeepRelativeTransform);

		ShieldCollisionComp = NewObject<USphereComponent>(Owner, TEXT("ShieldCollisionComp"));
		ShieldCollisionComp->RegisterComponent();
		ShieldCollisionComp->AttachToComponent(ShieldRootComp, FAttachmentTransformRules::KeepRelativeTransform);
		ShieldCollisionComp->InitSphereRadius(ShieldSphereRadius);

		ShieldCollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ShieldCollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
		ShieldCollisionComp->SetGenerateOverlapEvents(false);
		ShieldCollisionComp->SetNotifyRigidBodyCollision(false);

		ShieldCollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		ShieldCollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
		// (optional later) custom projectile channel

		ShieldVisualComp = NewObject<UStaticMeshComponent>(Owner, TEXT("ShieldVisualComp"));
		ShieldVisualComp->RegisterComponent();
		ShieldVisualComp->AttachToComponent(ShieldRootComp, FAttachmentTransformRules::KeepRelativeTransform);
		ShieldVisualComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ShieldVisualComp->SetHiddenInGame(true, true);

		if (UStaticMesh* Sphere = GetEngineSphereMesh())
		{
			ShieldVisualComp->SetStaticMesh(Sphere);
		}

		if (ShieldMaterial)
		{
			ShieldVisualComp->SetMaterial(0, ShieldMaterial);
		}

		ShieldCollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ULGSShieldComponent::OnShieldBeginOverlap);
	}
	
	UpdateEligibilityAndVisuals();
}


//new 1_14_@6
void ULGSShieldComponent::OnShieldBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || !IsShieldActiveAndPowered()) return;

	if (!bDestroyBulletsOnOverlap) return;

	bool bLooksLikeBullet = false;

	if (BulletClass)
	{
		bLooksLikeBullet = OtherActor->IsA(BulletClass);
	}
	else
	{
		bLooksLikeBullet = OtherActor->GetName().Contains(TEXT("Bullet"));
	}

	if (bLooksLikeBullet)
	{
		OtherActor->Destroy();

		ShieldEnergy = FMath::Max(0.f, ShieldEnergy - BulletImpactCost);
		if (ShieldEnergy <= 0.f)
		{
			BreakShield();
			return; // BreakShield already refreshed visuals/collision
		}

		UpdateEligibilityAndVisuals();
	}
}



//end 1_14_26

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
	// If shield isn't eligible, all damage passes through unchanged
	if (!IsShieldActiveAndPowered() || DamageAmount <= 0.f)
	{
		return DamageAmount;
	}

	const float Absorb = FMath::Min(ShieldEnergy, DamageAmount);
	ShieldEnergy -= Absorb;

	// If we hit 0, shield breaks and any leftover damage passes through
	if (ShieldEnergy <= 0.f)
	{
		BreakShield();
		return DamageAmount - Absorb;
	}

	// Shield is still up; just refresh visuals/collision state
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
	// (keep your eligibility logic if you want, or skip for now)

	const bool bShow = bShieldActive && !bShieldBroken && ShieldEnergy > 0.f;

	if (ShieldVisualComp)
	{
		ShieldVisualComp->SetHiddenInGame(!bShow, true);
	}

	if (ShieldCollisionComp)
	{
		if (bShow)
		{
			ShieldCollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			ShieldCollisionComp->SetGenerateOverlapEvents(true);
		}
		else
		{
			ShieldCollisionComp->SetGenerateOverlapEvents(false);
			ShieldCollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

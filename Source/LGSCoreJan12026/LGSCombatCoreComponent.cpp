#include "LGSCombatCoreComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "LGSCoreJan12026Character.h"

ULGSCombatCoreComponent::ULGSCombatCoreComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULGSCombatCoreComponent::BeginPlay()
{
	Super::BeginPlay();
	OnCombatModeChanged.Broadcast(CombatMode);
}

void ULGSCombatCoreComponent::ToggleCombatMode()
{
	//ok

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Green,
			FString::Printf(
				TEXT("ToggleCombatMode pressed. Current=%s"),
				CombatMode == ECombatMode::Ranged ? TEXT("Ranged") : TEXT("Melee")
			)
		);
	}

	SetCombatMode(CombatMode == ECombatMode::Ranged ? ECombatMode::Melee : ECombatMode::Ranged);

	


}

void ULGSCombatCoreComponent::SetCombatMode(ECombatMode NewMode)
{

	UE_LOG(LogTemplateCharacter, Warning, TEXT("[MODE] SetCombatMode %s -> %s"),
	CombatMode == ECombatMode::Ranged ? TEXT("Ranged") : TEXT("Melee"),
	NewMode == ECombatMode::Ranged ? TEXT("Ranged") : TEXT("Melee"));

	if (CombatMode == NewMode) return;

	CombatMode = NewMode;
	OnCombatModeChanged.Broadcast(CombatMode);

	UE_LOG(LogTemplateCharacter, Warning, TEXT("[MODE] SetCombatMode %s -> %s"),
	CombatMode == ECombatMode::Ranged ? TEXT("Ranged") : TEXT("Melee"),
	NewMode == ECombatMode::Ranged ? TEXT("Ranged") : TEXT("Melee"));

}

void ULGSCombatCoreComponent::TryPrimary()
{
	if (CombatMode == ECombatMode::Ranged)
	{
		DoRangedShot();
	}
	else
	{
		DoMeleeSwing();
	}
}

void ULGSCombatCoreComponent::TrySecondary()
{
	// Reserved for Aim / Block / AltFire later.
}

void ULGSCombatCoreComponent::DoRangedShot()
{
	if (!bCanFire) return;
	bCanFire = false;

	UWorld* World = GetWorld();
	if (!World) { ResetFire(); return; }

	ALGSCoreJan12026Character* OwnerChar = Cast<ALGSCoreJan12026Character>(GetOwner());
	if (!OwnerChar) { ResetFire(); return; }

	if (!BulletClass)
	{
		World->GetTimerManager().SetTimer(Timer_FireCooldown, this, &ULGSCombatCoreComponent::ResetFire, FireCooldown, false);
		return;
	}
	//change to make rider build again to fix desync issue
	//new 1_3_26
	// Prefer muzzle socket location, but aim with camera rotation for FPS feel.
	// Always initialize to something valid.
	FTransform SpawnXform = OwnerChar->GetActorTransform();

	UCameraComponent* Cam = OwnerChar->GetFirstPersonCameraComponent();
	const FRotator AimRot = Cam ? Cam->GetComponentRotation() : OwnerChar->GetActorRotation();

	if (USkeletalMeshComponent* Mesh1P = OwnerChar->GetMesh1P())
	{
		if (Mesh1P->DoesSocketExist(MuzzleSocketName))
		{
			// Use socket location, but aim where the camera looks
			const FVector MuzzleLoc = Mesh1P->GetSocketLocation(MuzzleSocketName);
			SpawnXform = FTransform(AimRot, MuzzleLoc);
		}
		else if (Cam)
		{
			// No socket: spawn a bit in front of camera
			const FVector Loc = Cam->GetComponentLocation() + Cam->GetForwardVector() * 100.f;
			SpawnXform = FTransform(AimRot, Loc);
		}
	}
	else if (Cam)
	{
		// Mesh missing: still allow shooting
		const FVector Loc = Cam->GetComponentLocation() + Cam->GetForwardVector() * 100.f;
		SpawnXform = FTransform(AimRot, Loc);
	}
	//end 1_3_26

	FActorSpawnParameters Params;
	Params.Owner = OwnerChar;
	Params.Instigator = OwnerChar;

	World->SpawnActor<AActor>(BulletClass, SpawnXform, Params);

	World->GetTimerManager().SetTimer(Timer_FireCooldown, this, &ULGSCombatCoreComponent::ResetFire, FireCooldown, false);
}

void ULGSCombatCoreComponent::DoMeleeSwing()
{
	if (!bCanMelee) return;

	UWorld* World = GetWorld();
	if (!World) return;

	bCanMelee = false;

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar)
	{
		ResetMelee();
		return;
	}

	if (MeleeMontage)
	{
		OwnerChar->PlayAnimMontage(MeleeMontage);
	}

	World->GetTimerManager().SetTimer(
		Timer_MeleeCooldown, this, &ULGSCombatCoreComponent::ResetMelee, MeleeCooldown, false);
}

void ULGSCombatCoreComponent::ResetFire()
{
	bCanFire = true;
}

void ULGSCombatCoreComponent::ResetMelee()
{
	bCanMelee = true;
}

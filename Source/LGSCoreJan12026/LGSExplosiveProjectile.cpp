#include "LGSExplosiveProjectile.h"

#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Pawn.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Engine/EngineTypes.h"

ALGSExplosiveProjectile::ALGSExplosiveProjectile()
{
	InitialLifeSpan = 2.0f;
}

void ALGSExplosiveProjectile::HandleImpact(
	const FHitResult& Hit,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp)
{
	if (bHasExploded)
	{
		return;
	}

	bHasExploded = true;

	// Let BP still decide AI blood / impact FX / hit confirm logic if desired.
	BP_OnProjectileImpact(Hit, OtherActor, OtherComp);

	const FVector ImpactPoint = Hit.bBlockingHit ? FVector(Hit.ImpactPoint) : GetActorLocation();

	Explode(ImpactPoint);
	Destroy();
}

void ALGSExplosiveProjectile::Explode(const FVector& ImpactPoint)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (ExplosionFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			ExplosionFX,
			ImpactPoint,
			FRotator::ZeroRotator
		);
	}

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);

	if (AActor* OwnerActor = GetOwner())
	{
		IgnoreActors.Add(OwnerActor);
	}

	AController* InstigatorController = nullptr;
	if (APawn* InstigatorPawn = GetInstigator())
	{
		InstigatorController = InstigatorPawn->GetController();
	}

	if (ExplosionDamage > 0.f && ExplosionRadius > 0.f)
	{
		UGameplayStatics::ApplyRadialDamage(
			World,
			ExplosionDamage,
			ImpactPoint,
			ExplosionRadius,
			nullptr,
			IgnoreActors,
			this,
			InstigatorController,
			true
		);
	}

	if (ExplosionImpulse > 0.f && ExplosionRadius > 0.f)
	{
		TArray<FHitResult> Hits;
		FCollisionShape Sphere = FCollisionShape::MakeSphere(ExplosionRadius);
		FCollisionQueryParams Params(SCENE_QUERY_STAT(ExplosiveProjectileImpulse), false);
		Params.AddIgnoredActor(this);

		if (AActor* OwnerActor = GetOwner())
		{
			Params.AddIgnoredActor(OwnerActor);
		}

		const bool bAnyHit = World->SweepMultiByChannel(
			Hits,
			ImpactPoint,
			ImpactPoint,
			FQuat::Identity,
			ECC_PhysicsBody,
			Sphere,
			Params
		);

		if (bAnyHit)
		{
			for (const FHitResult& Result : Hits)
			{
				UPrimitiveComponent* Prim = Result.GetComponent();
				if (!Prim || !Prim->IsSimulatingPhysics())
				{
					continue;
				}

				const FVector Dir = (Prim->GetComponentLocation() - ImpactPoint).GetSafeNormal();
				Prim->AddImpulseAtLocation(Dir * ExplosionImpulse, Result.ImpactPoint);
			}
		}
	}
}
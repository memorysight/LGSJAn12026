#pragma once

#include "CoreMinimal.h"
#include "LGSCoreJan12026Projectile.h"
#include "LGSExplosiveProjectile.generated.h"

class UNiagaraSystem;
class UPrimitiveComponent;

UCLASS()
class LGSCOREJAN12026_API ALGSExplosiveProjectile : public ALGSCoreJan12026Projectile
{
	GENERATED_BODY()

public:
	ALGSExplosiveProjectile();

protected:
	virtual void HandleImpact(
		const FHitResult& Hit,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp
	) override;

	void Explode(const FVector& ImpactPoint);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Explosion", meta=(ClampMin="0.0"))
	float ExplosionRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Explosion", meta=(ClampMin="0.0"))
	float ExplosionDamage = 35.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Explosion", meta=(ClampMin="0.0"))
	float ExplosionImpulse = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Explosion|FX")
	TObjectPtr<UNiagaraSystem> ExplosionFX = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Explosion")
	bool bHasExploded = false;
};
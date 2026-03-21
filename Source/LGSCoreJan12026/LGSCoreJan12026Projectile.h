#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "LGSCoreJan12026Projectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UDamageType;

UCLASS(config=Game)
class LGSCOREJAN12026_API ALGSCoreJan12026Projectile : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Movement, meta=(AllowPrivateAccess="true"))
	UProjectileMovementComponent* ProjectileMovement;

public:
	ALGSCoreJan12026Projectile();

	UFUNCTION()
	void OnHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

	USphereComponent* GetCollisionComp() const { return CollisionComp; }
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage", meta=(ClampMin="0.0"))
	float DirectHitDamage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage")
	TSubclassOf<UDamageType> DamageTypeClass = nullptr;

	// C++ owns damage / destroy. BP owns impact FX decisions like blood, sparks, decals.
	UFUNCTION(BlueprintImplementableEvent, Category="Projectile|Impact")
	void BP_OnProjectileImpact(
		const FHitResult& Hit,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp
	);

	virtual void HandleImpact(
		const FHitResult& Hit,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp
	);
};
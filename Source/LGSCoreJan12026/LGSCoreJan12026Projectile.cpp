#include "LGSCoreJan12026Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"

ALGSCoreJan12026Projectile::ALGSCoreJan12026Projectile()
{
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &ALGSCoreJan12026Projectile::OnHit);

	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	RootComponent = CollisionComp;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3777.f;
	ProjectileMovement->MaxSpeed = 3777.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	InitialLifeSpan = 3.0f;
}

void ALGSCoreJan12026Projectile::BeginPlay()
{
	Super::BeginPlay();

	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = GetActorForwardVector() * ProjectileMovement->InitialSpeed;
	}
}

void ALGSCoreJan12026Projectile::OnHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	HandleImpact(Hit, OtherActor, OtherComp);
}

void ALGSCoreJan12026Projectile::HandleImpact(
	const FHitResult& Hit,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp)
{
	AController* InstigatorController = nullptr;
	if (APawn* InstigatorPawn = GetInstigator())
	{
		InstigatorController = InstigatorPawn->GetController();
	}

	if (OtherActor && OtherActor != this && OtherActor != GetOwner())
	{
		if (DirectHitDamage > 0.0f)
		{
			UGameplayStatics::ApplyPointDamage(
				OtherActor,
				DirectHitDamage,
				GetVelocity().GetSafeNormal(),
				Hit,
				InstigatorController,
				GetOwner(),          // important: often better for downstream hit logic than 'this'
				DamageTypeClass
			);
		}
	}

	// Important: run BP impact FX before destroy so blood/hit confirm logic still fires.
	BP_OnProjectileImpact(Hit, OtherActor, OtherComp);

	if (OtherComp && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
	}

	Destroy();
}
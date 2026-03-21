#include "LGSCoreJan12026Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
//new 3_21 explosive bullets make normal work
#include "Kismet/GameplayStatics.h"
//end 3_21
#include "Components/SphereComponent.h"

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

//new 3_21 Make normal bullets work
void ALGSCoreJan12026Projectile::HandleImpact(
	const FHitResult& Hit,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp)
{
	if (OtherActor && OtherActor != this && OtherActor != GetOwner())
	{
		AController* InstigatorController = nullptr;
		if (APawn* InstigatorPawn = GetInstigator())
		{
			InstigatorController = InstigatorPawn->GetController();
		}

		if (DirectHitDamage > 0.0f)
		{
			UGameplayStatics::ApplyPointDamage(
				OtherActor,
				DirectHitDamage,
				GetVelocity().GetSafeNormal(),
				Hit,
				InstigatorController,
				this,
				nullptr
			);
		}
	}

	if (OtherComp && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
	}

	Destroy();
}
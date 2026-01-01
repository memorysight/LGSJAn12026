// Copyright Epic Games, Inc. All Rights Reserved.

#include "LGSCoreJan12026PickUpComponent.h"

ULGSCoreJan12026PickUpComponent::ULGSCoreJan12026PickUpComponent()
{
	// Setup the Sphere Collision
	SphereRadius = 32.f;
}

void ULGSCoreJan12026PickUpComponent::BeginPlay()
{
	Super::BeginPlay();

	// Register our Overlap Event
	OnComponentBeginOverlap.AddDynamic(this, &ULGSCoreJan12026PickUpComponent::OnSphereBeginOverlap);
}

void ULGSCoreJan12026PickUpComponent::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Checking if it is a First Person Character overlapping
	ALGSCoreJan12026Character* Character = Cast<ALGSCoreJan12026Character>(OtherActor);
	if(Character != nullptr)
	{
		// Notify that the actor is being picked up
		OnPickUp.Broadcast(Character);

		// Unregister from the Overlap Event so it is no longer triggered
		OnComponentBeginOverlap.RemoveAll(this);
	}
}

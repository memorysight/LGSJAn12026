#include "LGSMVSComponent.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"


ULGSMVSComponent::ULGSMVSComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void ULGSMVSComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("LGS MVS Component Online"));
}


void ULGSMVSComponent::StartMusic()
{
	if (!BaseComposition)
	{
		UE_LOG(LogTemp, Warning, TEXT("MVS: No BaseComposition assigned."));
		return;
	}

	if (BaseAudioComponent && BaseAudioComponent->IsPlaying())
	{
		return;
	}

	BaseAudioComponent = UGameplayStatics::SpawnSound2D(
		this,
		BaseComposition
	);

	if (BaseAudioComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("MVS: Base composition started."));
	}
}


void ULGSMVSComponent::StopMusic()
{
	if (!BaseAudioComponent)
	{
		return;
	}

	BaseAudioComponent->Stop();
	BaseAudioComponent = nullptr;

	UE_LOG(LogTemp, Log, TEXT("MVS: Base composition stopped."));
}
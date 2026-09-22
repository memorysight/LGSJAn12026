#include "LGSMVSComponent.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"




//2nd round of edits
namespace
{
	constexpr float MVSStemSilentVolume = 0.001f;
}

ULGSMVSComponent::ULGSMVSComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void ULGSMVSComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(
		LogTemp,
		Log,
		TEXT("LGS MVS Component Online")
	);
}


// ============================================================
// STANDALONE MUSIC
// ============================================================

void ULGSMVSComponent::PlayStandaloneComposition()
{
	if (!StandaloneComposition)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("MVS: No StandaloneComposition assigned.")
		);

		return;
	}


	// Do not play standalone music over an MVS composition.
	if (bMVSPlaying)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("MVS: Cannot start standalone composition while MVS is running.")
		);

		return;
	}


	if (StandaloneAudioComponent &&
		StandaloneAudioComponent->IsPlaying())
	{
		return;
	}


	StandaloneAudioComponent =
		UGameplayStatics::SpawnSound2D(
			this,
			StandaloneComposition,
			1.0f,
			1.0f,
			0.0f,
			nullptr,
			true,
			false
		);


	if (StandaloneAudioComponent)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("MVS: Standalone composition started.")
		);
	}
}


void ULGSMVSComponent::StopStandaloneComposition()
{
	if (!StandaloneAudioComponent)
	{
		return;
	}


	StandaloneAudioComponent->Stop();

	StandaloneAudioComponent = nullptr;


	UE_LOG(
		LogTemp,
		Log,
		TEXT("MVS: Standalone composition stopped.")
	);
}


// ============================================================
// START MVS
// ============================================================

void ULGSMVSComponent::StartMusic()
{
	if (bMVSPlaying)
	{
		return;
	}

	if (StandaloneAudioComponent)
	{
		StandaloneAudioComponent->Stop();
		StandaloneAudioComponent = nullptr;
	}

	if (!EmptyDisclosuresBaseStem)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("MVS: Empty Disclosures Base Stem is not assigned.")
		);

		return;
	}

	BaseStemAudioComponent =
		UGameplayStatics::SpawnSound2D(
			this,
			EmptyDisclosuresBaseStem,
			BaseStemVolume,
			1.0f,
			0.0f,
			nullptr,
			true,
			false
		);

	if (EmptyDisclosuresCrouchStem)
	{
		CrouchStemAudioComponent =
			UGameplayStatics::SpawnSound2D(
				this,
				EmptyDisclosuresCrouchStem,
				
				//Tiny 'keep-alive' level
				//9_22_2nd round actually use 'keep alive value from 0.0f to 1.0f
				//Effectively inaudible, BUT avoids starting the voice at exact zero
				MVSStemSilentVolume,
				1.0f,
				0.0f,
				nullptr,
				true,
				false
			);

		//9_13 establish timeline
		UE_LOG(
		LogTemp,
		Warning,
		TEXT("MVS CROUCH SPAWN: Component=%s IsPlaying=%d Volume=%.3f"),
		*GetNameSafe(CrouchStemAudioComponent),
		CrouchStemAudioComponent &&
		CrouchStemAudioComponent->IsPlaying() ? 1 : 0,
		CrouchStemAudioComponent
			? CrouchStemAudioComponent->VolumeMultiplier
			: -1.0f
			);

	}

	//new 9_22 shield
	//9_22_2nd round actually use 'keep alive value from 0.0f to 1.0f
	if (EmptyDisclosuresShieldStem)
	{
		ShieldStemAudioComponent =
			UGameplayStatics::SpawnSound2D(
				this,
				EmptyDisclosuresShieldStem,
				MVSStemSilentVolume,
				1.0f,
				0.0f,
				nullptr,
				true,
				false
			);

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("MVS SHIELD SPAWN: Component=%s IsPlaying=%d Volume=%.3f"),
			*GetNameSafe(ShieldStemAudioComponent),
			ShieldStemAudioComponent &&
			ShieldStemAudioComponent->IsPlaying() ? 1 : 0,
			ShieldStemAudioComponent
				? ShieldStemAudioComponent->VolumeMultiplier
				: -1.0f
		);
	}

	
	bMVSPlaying = true;
	bCrouchStemAudible = false;
	//new 9_22 shield
	bShieldStemAudible = false;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("MVS MVP2 STARTED - Base + Crouch + Shield stems.  New Sonic Landscapes Explored!!")
	);
}


// ============================================================
// STOP MVS
// ============================================================

void ULGSMVSComponent::StopMVS()
{
	if (BaseStemAudioComponent)
	{
		BaseStemAudioComponent->Stop();
		BaseStemAudioComponent = nullptr;
	}


	if (CrouchStemAudioComponent)
	{
		CrouchStemAudioComponent->Stop();
		CrouchStemAudioComponent = nullptr;
	}

	//new 9_22 shield
	if (ShieldStemAudioComponent)
	{
		ShieldStemAudioComponent->Stop();
		ShieldStemAudioComponent = nullptr;
	}


	bMVSPlaying = false;
	bCrouchStemAudible = false;
	//new 9_22 shield
	bShieldStemAudible = false;


	UE_LOG(
		LogTemp,
		Log,
		TEXT("MVS stopped.")
	);
}


bool ULGSMVSComponent::IsMVSPlaying() const
{
	return bMVSPlaying;
}


// ============================================================
// CROUCH
// ============================================================

void ULGSMVSComponent::SetCrouchStemActive(bool bActive)
{
	//new 9_12
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("MVS SetCrouchStemActive called: %s"),
			bActive ? TEXT("TRUE") : TEXT("FALSE")
		);

	
	if (!bMVSPlaying)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("MVS: Crouch state received before MVS started.")
		);

		return;
	}


	if (!CrouchStemAudioComponent)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("MVS: No Crouch Stem Audio Component.")
		);

		return;
	}


	// Don't repeatedly issue identical volume commands.
	if (bCrouchStemAudible == bActive)
	{
		return;
	}


	bCrouchStemAudible = bActive;


	if (bActive)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("MVS CROUCH before unmute: IsPlaying=%d Volume=%.3f"),
			CrouchStemAudioComponent->IsPlaying() ? 1 : 0,
			CrouchStemAudioComponent->VolumeMultiplier
		);

		// TEMP MVP1 DIAGNOSTIC:
		// Instant mixer change. Does NOT restart playback.
		CrouchStemAudioComponent->SetVolumeMultiplier(
			CrouchStemVolume
		);

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("MVS CROUCH AFTER SetVolumeMultiplier: IsPlaying=%d Volume=%.3f Target=%.3f"),
			CrouchStemAudioComponent->IsPlaying() ? 1 : 0,
			CrouchStemAudioComponent->VolumeMultiplier,
			CrouchStemVolume
		);

		UE_LOG(
			LogTemp,
			Log,
			TEXT("MVS: CROUCH STEM AUDIBLE")
		);
	}
	//new 9_22 2nd Round edits change from 0.0f to MVSStemSilentVolume
	else
	{
		CrouchStemAudioComponent->SetVolumeMultiplier(MVSStemSilentVolume);

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("MVS CROUCH AFTER mute: IsPlaying=%d Volume=%.3f"),
			CrouchStemAudioComponent->IsPlaying() ? 1 : 0,
			CrouchStemAudioComponent->VolumeMultiplier
		);

		UE_LOG(
			LogTemp,
			Log,
			TEXT("MVS: CROUCH STEM MUTED")
		);
	}
}

//new 9_22
// ============================================================
// SHIELD
// ============================================================

void ULGSMVSComponent::SetShieldStemActive(bool bActive)
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("MVS SetShieldStemActive called: %s"),
        bActive ? TEXT("TRUE") : TEXT("FALSE")
    );

    if (!bMVSPlaying)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS: Shield state received before MVS started.")
        );

        return;
    }

    if (!ShieldStemAudioComponent)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS: No Shield Stem Audio Component.")
        );

        return;
    }

    // Don't repeatedly issue identical volume commands.
    if (bShieldStemAudible == bActive)
    {
        return;
    }

    bShieldStemAudible = bActive;

    if (bActive)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS SHIELD before unmute: IsPlaying=%d Volume=%.3f"),
            ShieldStemAudioComponent->IsPlaying() ? 1 : 0,
            ShieldStemAudioComponent->VolumeMultiplier
        );

        // Instant mixer change.
        // Does NOT restart playback.
        ShieldStemAudioComponent->SetVolumeMultiplier(
            ShieldStemVolume
        );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS SHIELD AFTER SetVolumeMultiplier: IsPlaying=%d Volume=%.3f Target=%.3f"),
            ShieldStemAudioComponent->IsPlaying() ? 1 : 0,
            ShieldStemAudioComponent->VolumeMultiplier,
            ShieldStemVolume
        );

        UE_LOG(
            LogTemp,
            Log,
            TEXT("MVS: SHIELD STEM AUDIBLE")
        );
    }
	//9_22_2nd round changed from 0.0f to MVSStemSilentVolume
    else
    {
        ShieldStemAudioComponent->SetVolumeMultiplier(MVSStemSilentVolume);

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS SHIELD AFTER mute: IsPlaying=%d Volume=%.3f"),
            ShieldStemAudioComponent->IsPlaying() ? 1 : 0,
            ShieldStemAudioComponent->VolumeMultiplier
        );

        UE_LOG(
            LogTemp,
            Log,
            TEXT("MVS: SHIELD STEM MUTED")
        );
    }
}


bool ULGSMVSComponent::IsCrouchStemActive() const
{
	return bCrouchStemAudible;
}

//new 9_22
bool ULGSMVSComponent::IsShieldStemActive() const
{
	return bShieldStemAudible;
}



// ============================================================
// DEBUG
// ============================================================

void ULGSMVSComponent::DebugPrintMVSState() const
{
	UE_LOG(
		LogTemp,
		Log,
		TEXT("========== MVS MVP1 ==========")
	);


	UE_LOG(
		LogTemp,
		Log,
		TEXT("MVS Running: %s"),
		bMVSPlaying
			? TEXT("YES")
			: TEXT("NO")
	);


	UE_LOG(
		LogTemp,
		Log,
		TEXT("Base Stem: %s"),
		BaseStemAudioComponent &&
		BaseStemAudioComponent->IsPlaying()
			? TEXT("PLAYING")
			: TEXT("NOT PLAYING")
	);


	UE_LOG(
		LogTemp,
		Log,
		TEXT("Crouch Stem Playback: %s"),
		CrouchStemAudioComponent &&
		CrouchStemAudioComponent->IsPlaying()
			? TEXT("PLAYING")
			: TEXT("NOT PLAYING")
	);


	UE_LOG(
		LogTemp,
		Log,
		TEXT("Crouch Stem Audible: %s"),
		bCrouchStemAudible
			? TEXT("YES")
			: TEXT("NO")
	);


	UE_LOG(
		LogTemp,
		Log,
		TEXT("==============================")
	);
}
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

	//9_22 shield
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

	// new 9_25 AIRWALKING!!! Breaking the SoundBarrier
	if (EmptyDisclosuresAirWalkStem)
	{
		AirWalkStemAudioComponent =
			UGameplayStatics::SpawnSound2D(
				this,
				EmptyDisclosuresAirWalkStem,
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
			TEXT("MVS AIRWALK SPAWN: Component=%s IsPlaying=%d Volume=%.3f"),
			*GetNameSafe(AirWalkStemAudioComponent),
			AirWalkStemAudioComponent &&
			AirWalkStemAudioComponent->IsPlaying() ? 1 : 0,
			AirWalkStemAudioComponent
				? AirWalkStemAudioComponent->VolumeMultiplier
				: -1.0f
		);
	}

	// new 9_27 OverDrive Killing Streak!!! Let the Melee Shock the Enemies into Submission! 
	if (EmptyDisclosuresOverDriveStem)
	{
		OverDriveStemAudioComponent =
			UGameplayStatics::SpawnSound2D(
				this,
				EmptyDisclosuresOverDriveStem,
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
			TEXT("MVS OVERDRIVE SPAWN: Component=%s IsPlaying=%d Volume=%.3f"),
			*GetNameSafe(OverDriveStemAudioComponent),
			OverDriveStemAudioComponent &&
			OverDriveStemAudioComponent->IsPlaying() ? 1 : 0,
			OverDriveStemAudioComponent
				? OverDriveStemAudioComponent->VolumeMultiplier
				: -1.0f
		);
	}

	// 9_28 HyperDrive Killing Streak
	if (EmptyDisclosuresHyperDriveStem)
	{
		HyperDriveStemAudioComponent =
			UGameplayStatics::SpawnSound2D(
				this,
				EmptyDisclosuresHyperDriveStem,
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
			TEXT("MVS HYPERDRIVE SPAWN: Component=%s IsPlaying=%d Volume=%.3f"),
			*GetNameSafe(HyperDriveStemAudioComponent),
			HyperDriveStemAudioComponent &&
			HyperDriveStemAudioComponent->IsPlaying() ? 1 : 0,
			HyperDriveStemAudioComponent
				? HyperDriveStemAudioComponent->VolumeMultiplier
				: -1.0f
		);
	}

	//9_27 OverDrive
	bOverDriveStemAudible = false;
	//new 9_28 HyperDrive
	bHyperDriveStemAudible = false;

	
	bMVSPlaying = true;
	bCrouchStemAudible = false;
	//9_22 shield
	bShieldStemAudible = false;
	//9_25
	bAirWalkStemAudible = false;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("MVS MVP5 STARTED - Base + Crouch + Shield + AirWalk + OverDrive + HyperDrive. Unleash the first fully player enabled synth!!")
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

	//9_22 shield
	if (ShieldStemAudioComponent)
	{
		ShieldStemAudioComponent->Stop();
		ShieldStemAudioComponent = nullptr;
	}
	//9_25_ stop AIRWALKING LIKE A BOSS
	if (AirWalkStemAudioComponent)
	{
		AirWalkStemAudioComponent->Stop();
		AirWalkStemAudioComponent = nullptr;
	}
	//9_27 stop OverDrive Devastation! 
	if (OverDriveStemAudioComponent)
	{
		OverDriveStemAudioComponent->Stop();
		OverDriveStemAudioComponent = nullptr;
	}
	// 9_28 stop HyperDrive
	if (HyperDriveStemAudioComponent)
	{
		HyperDriveStemAudioComponent->Stop();
		HyperDriveStemAudioComponent = nullptr;
	}


	bMVSPlaying = false;
	bCrouchStemAudible = false;
	//9_22 shield
	bShieldStemAudible = false;
	//new 9_25
	bAirWalkStemAudible = false;
	//new 9_27 ending Melee Onslaught
	bOverDriveStemAudible = false;
	//new 9_28 Watch the countesses cower in fear as they hear the sounds of a million angels singing in harmony
	bHyperDriveStemAudible = false;


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
	//9_12
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
	//9_22 2nd Round edits change from 0.0f to MVSStemSilentVolume
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

//9_22
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

// 9_25====================================================
// AIRWALK
// ============================================================

void ULGSMVSComponent::SetAirWalkStemActive(bool bActive)
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("MVS SetAirWalkStemActive called: %s"),
        bActive ? TEXT("TRUE") : TEXT("FALSE")
    );

    if (!bMVSPlaying)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS: AirWalk state received before MVS started.")
        );

        return;
    }

    if (!AirWalkStemAudioComponent)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS: No AirWalk Stem Audio Component.")
        );

        return;
    }

    // Don't repeatedly issue identical volume commands.
    if (bAirWalkStemAudible == bActive)
    {
        return;
    }

    bAirWalkStemAudible = bActive;

    if (bActive)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS AIRWALK before unmute: IsPlaying=%d Volume=%.3f"),
            AirWalkStemAudioComponent->IsPlaying() ? 1 : 0,
            AirWalkStemAudioComponent->VolumeMultiplier
        );

        // Instant mixer change.
        // Does NOT restart playback.
        AirWalkStemAudioComponent->SetVolumeMultiplier(
            AirWalkStemVolume
        );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS AIRWALK AFTER SetVolumeMultiplier: IsPlaying=%d Volume=%.3f Target=%.3f"),
            AirWalkStemAudioComponent->IsPlaying() ? 1 : 0,
            AirWalkStemAudioComponent->VolumeMultiplier,
            AirWalkStemVolume
        );

        UE_LOG(
            LogTemp,
            Log,
            TEXT("MVS: AIRWALK STEM AUDIBLE")
        );
    }
    else
    {
        AirWalkStemAudioComponent->SetVolumeMultiplier(
            MVSStemSilentVolume
        );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS AIRWALK AFTER mute: IsPlaying=%d Volume=%.3f"),
            AirWalkStemAudioComponent->IsPlaying() ? 1 : 0,
            AirWalkStemAudioComponent->VolumeMultiplier
        );

        UE_LOG(
            LogTemp,
            Log,
            TEXT("MVS: AIRWALK STEM MUTED")
        );
    }
}



// 9_25====================================================
// OverDrive EventHorizon and Melee Devastation 
// ============================================================

void ULGSMVSComponent::SetOverDriveStemActive(bool bActive)
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("MVS SetOverDriveStemActive CALLED: Active=%d MVSPlaying=%d Component=%s"),
        bActive ? 1 : 0,
        bMVSPlaying ? 1 : 0,
        *GetNameSafe(OverDriveStemAudioComponent)
    );

    if (!bMVSPlaying)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS OVERDRIVE ignored - MVS is not playing")
        );
        return;
    }

    if (!OverDriveStemAudioComponent)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS OVERDRIVE ignored - OverDriveStemAudioComponent is NULL")
        );
        return;
    }

    if (bOverDriveStemAudible == bActive)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS OVERDRIVE unchanged - already %s"),
            bActive ? TEXT("AUDIBLE") : TEXT("MUTED")
        );
        return;
    }

    bOverDriveStemAudible = bActive;

    if (bActive)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS OVERDRIVE before unmute: IsPlaying=%d Volume=%.3f"),
            OverDriveStemAudioComponent->IsPlaying() ? 1 : 0,
            OverDriveStemAudioComponent->VolumeMultiplier
        );

        OverDriveStemAudioComponent->SetVolumeMultiplier(
            OverDriveStemVolume
        );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS OVERDRIVE AUDIBLE: IsPlaying=%d Volume=%.3f Target=%.3f"),
            OverDriveStemAudioComponent->IsPlaying() ? 1 : 0,
            OverDriveStemAudioComponent->VolumeMultiplier,
            OverDriveStemVolume
        );
    }
    else
    {
        OverDriveStemAudioComponent->SetVolumeMultiplier(
            MVSStemSilentVolume
        );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS OVERDRIVE MUTED: IsPlaying=%d Volume=%.3f"),
            OverDriveStemAudioComponent->IsPlaying() ? 1 : 0,
            OverDriveStemAudioComponent->VolumeMultiplier
        );
    }
}

// new 9_25====================================================
// HYPERDRIVE : all those who dare attack will be cut to pieces as music echoes in the air
// ============================================================

void ULGSMVSComponent::SetHyperDriveStemActive(bool bActive)
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("MVS SetHyperDriveStemActive CALLED: Active=%d MVSPlaying=%d Component=%s"),
        bActive ? 1 : 0,
        bMVSPlaying ? 1 : 0,
        *GetNameSafe(HyperDriveStemAudioComponent)
    );

    if (!bMVSPlaying)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS HYPERDRIVE ignored - MVS is not playing")
        );
        return;
    }

    if (!HyperDriveStemAudioComponent)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS HYPERDRIVE ignored - HyperDriveStemAudioComponent is NULL")
        );
        return;
    }

    if (bHyperDriveStemAudible == bActive)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS HYPERDRIVE unchanged - already %s"),
            bActive ? TEXT("AUDIBLE") : TEXT("MUTED")
        );
        return;
    }

    bHyperDriveStemAudible = bActive;

    if (bActive)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS HYPERDRIVE before unmute: IsPlaying=%d Volume=%.3f"),
            HyperDriveStemAudioComponent->IsPlaying() ? 1 : 0,
            HyperDriveStemAudioComponent->VolumeMultiplier
        );

        HyperDriveStemAudioComponent->SetVolumeMultiplier(
            HyperDriveStemVolume
        );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS HYPERDRIVE AUDIBLE: IsPlaying=%d Volume=%.3f Target=%.3f"),
            HyperDriveStemAudioComponent->IsPlaying() ? 1 : 0,
            HyperDriveStemAudioComponent->VolumeMultiplier,
            HyperDriveStemVolume
        );
    }
    else
    {
        HyperDriveStemAudioComponent->SetVolumeMultiplier(
            MVSStemSilentVolume
        );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MVS HYPERDRIVE MUTED: IsPlaying=%d Volume=%.3f"),
            HyperDriveStemAudioComponent->IsPlaying() ? 1 : 0,
            HyperDriveStemAudioComponent->VolumeMultiplier
        );
    }
}

bool ULGSMVSComponent::IsHyperDriveStemActive() const
{
    return bHyperDriveStemAudible;
}

bool ULGSMVSComponent::IsOverDriveStemActive() const
{
    return bOverDriveStemAudible;
}

bool ULGSMVSComponent::IsAirWalkStemActive() const
{
    return bAirWalkStemAudible;
}


bool ULGSMVSComponent::IsCrouchStemActive() const
{
	return bCrouchStemAudible;
}

//9_22
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
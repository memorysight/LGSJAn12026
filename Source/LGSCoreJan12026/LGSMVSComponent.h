#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LGSMVSComponent.generated.h"

class USoundBase;
class UAudioComponent;


UCLASS(ClassGroup=(LGS), meta=(BlueprintSpawnableComponent))
class LGSCOREJAN12026_API ULGSMVSComponent : public UActorComponent
{
	GENERATED_BODY()


public:

	ULGSMVSComponent();


protected:

	virtual void BeginPlay() override;


	// ========================================================
	// STANDALONE MUSIC
	//
	// This is NOT part of MVS.
	//
	// Use this when a level simply wants to play one complete
	// bounced composition.
	// ========================================================

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category="MVS|Standalone"
	)
	TObjectPtr<USoundBase> StandaloneComposition = nullptr;


	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> StandaloneAudioComponent = nullptr;



	// ========================================================
	// EMPTY DISCLOSURES - MVP1
	// ========================================================

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category="MVS|Empty Disclosures"
	)
	TObjectPtr<USoundBase> EmptyDisclosuresBaseStem = nullptr;


	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category="MVS|Empty Disclosures"
	)
	TObjectPtr<USoundBase> EmptyDisclosuresCrouchStem = nullptr;

	//new 9_22 Shield

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category="MVS|Empty Disclosures"
)
	TObjectPtr<USoundBase> EmptyDisclosuresShieldStem = nullptr;



	
	// ========================================================
	// RUNTIME MVS COMPONENTS
	// ========================================================

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> BaseStemAudioComponent = nullptr;


	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> CrouchStemAudioComponent = nullptr;

	//new 9_22

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> ShieldStemAudioComponent = nullptr;



	// ========================================================
	// MVP1 MIX SETTINGS
	// ========================================================

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category="MVS|Mix",
		meta=(ClampMin="0.0", ClampMax="1.0")
	)
	float BaseStemVolume = 1.0f;


	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category="MVS|Mix",
		meta=(ClampMin="0.0", ClampMax="1.0")
	)
	float CrouchStemVolume = 1.0f;

	//new 9_22 shield
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category="MVS|Mix",
		meta=(ClampMin="0.0", ClampMax="1.0")
)
	float ShieldStemVolume = 1.0f;


	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category="MVS|Mix",
		meta=(ClampMin="0.0")
	)
	float StemFadeTime = 0.15f;



private:

	bool bMVSPlaying = false;

	bool bCrouchStemAudible = false;

	//new 9_22 shield

	bool bShieldStemAudible = false;



public:

	// ========================================================
	// STANDALONE PLAYBACK
	// ========================================================

	UFUNCTION(BlueprintCallable, Category="MVS|Standalone")
	void PlayStandaloneComposition();


	UFUNCTION(BlueprintCallable, Category="MVS|Standalone")
	void StopStandaloneComposition();



	// ========================================================
	// MVS PLAYBACK
	// ========================================================

	UFUNCTION(BlueprintCallable, Category="MVS|Playback")
	void StartMusic();


	UFUNCTION(BlueprintCallable, Category="MVS|Playback")
	void StopMVS();


	UFUNCTION(BlueprintPure, Category="MVS|Playback")
	bool IsMVSPlaying() const;



	// ========================================================
	// MVP1 - CROUCH
	// ========================================================

	UFUNCTION(BlueprintCallable, Category="MVS|Abilities")
	void SetCrouchStemActive(bool bActive);


	UFUNCTION(BlueprintPure, Category="MVS|Abilities")
	bool IsCrouchStemActive() const;

	//new 9_22

	// ========================================================
	// MVP2 - SHIELD
	// ========================================================

	UFUNCTION(BlueprintCallable, Category="MVS|Abilities")
	void SetShieldStemActive(bool bActive);

	UFUNCTION(BlueprintPure, Category="MVS|Abilities")
	bool IsShieldStemActive() const;



	// ========================================================
	// DEBUG
	// ========================================================

	UFUNCTION(BlueprintCallable, Category="MVS|Debug")
	void DebugPrintMVSState() const;
};
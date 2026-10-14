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

	//9_22 Shield

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category="MVS|Empty Disclosures"
)
	TObjectPtr<USoundBase> EmptyDisclosuresShieldStem = nullptr;

	//9_27 OverDrive
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category="MVS|Empty Disclosures")
	
	TObjectPtr<USoundBase> EmptyDisclosuresOverDriveStem = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> OverDriveStemAudioComponent = nullptr;

	
	// 9_28 HyperDrive
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category="MVS|Empty Disclosures"
	)
	TObjectPtr<USoundBase> EmptyDisclosuresHyperDriveStem = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> HyperDriveStemAudioComponent = nullptr;

	//new 9_30 OmegaDrive Integration: From alpha/crouch leads to Omega
	UPROPERTY(
	EditAnywhere,
	BlueprintReadOnly,
	Category="MVS|Empty Disclosures"
)
	TObjectPtr<USoundBase> EmptyDisclosuresOmegaDriveStem = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> OmegaDriveStemAudioComponent = nullptr;
	

	
	// ========================================================
	// RUNTIME MVS COMPONENTS
	// ========================================================

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> BaseStemAudioComponent = nullptr;


	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> CrouchStemAudioComponent = nullptr;

	//9_22

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> ShieldStemAudioComponent = nullptr;

	// 9_25 AIRWALK!!
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> AirWalkStemAudioComponent = nullptr;



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

	//9_22 shield
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category="MVS|Mix",
		meta=(ClampMin="0.0", ClampMax="1.0")
	)
	float ShieldStemVolume = 1.0f;

	//9_25 AIRWALK!!
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category="MVS|Empty Disclosures"
	)
	TObjectPtr<USoundBase> EmptyDisclosuresAirWalkStem = nullptr;

	// 9_25 AirWalk!!
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category="MVS|Mix",
		meta=(ClampMin="0.0", ClampMax="1.0")
	)
	float AirWalkStemVolume = 1.0f;

	// 9_27 OverDrive
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category="MVS|Mix",
		meta=(ClampMin="0.0", ClampMax="1.0")
		)
	float OverDriveStemVolume = 1.0f;

	//9_27
	// 9_28 HyperDrive
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category="MVS|Mix",
		meta=(ClampMin="0.0", ClampMax="1.0")
	)
	float HyperDriveStemVolume = 1.0f;

	// 9_30 OmegaDrive 'with the great soul enclosed in the fifth light'
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category="MVS|Mix",
		meta=(ClampMin="0.0", ClampMax="1.0")
	)
	float OmegaDriveStemVolume = 1.0f;
	
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

	//9_22 shield

	bool bShieldStemAudible = false;

	// 9_25 AirWalk
	bool bAirWalkStemAudible = false;

	// 9_27
	bool bOverDriveStemAudible = false;

	//9_28_
	bool bHyperDriveStemAudible = false;

	//new 9_30 OmegaDrive 'but each word shall harbor an idea'
	bool bOmegaDriveStemAudible = false;



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

	//9_22

	// ========================================================
	// MVP2 - SHIELD
	// ========================================================

	UFUNCTION(BlueprintCallable, Category="MVS|Abilities")
	void SetShieldStemActive(bool bActive);

	UFUNCTION(BlueprintPure, Category="MVS|Abilities")
	bool IsShieldStemActive() const;


	// 9_25====================================================
	// MVP3 - AIRWALK
	// ========================================================

	UFUNCTION(BlueprintCallable, Category="MVS|Abilities")
	void SetAirWalkStemActive(bool bActive);

	UFUNCTION(BlueprintPure, Category="MVS|Abilities")
	bool IsAirWalkStemActive() const;

	// 9_27====================================================
	// MVP3 - OverDrive Killing Streak
	// ========================================================

	UFUNCTION(BlueprintCallable, Category="MVS|Abilities")
	void SetOverDriveStemActive(bool bActive);

	UFUNCTION(BlueprintPure, Category="MVS|Abilities")
	bool IsOverDriveStemActive() const;

	// ====9_28================================================
	// MVP5 - HyperDrive Killing Streak
	// ========================================================

	UFUNCTION(BlueprintCallable, Category="MVS|Abilities")
	void SetHyperDriveStemActive(bool bActive);

	UFUNCTION(BlueprintPure, Category="MVS|Abilities")
	bool IsHyperDriveStemActive() const;


	// new 9_30_====aptitude for sublimation is subject to
	// MVP6 - OMEGADRIVE
	// ===============================individual variation

	UFUNCTION(BlueprintCallable, Category="MVS|Abilities")
	void SetOmegaDriveStemActive(bool bActive);

	UFUNCTION(BlueprintPure, Category="MVS|Abilities")
	bool IsOmegaDriveStemActive() const;



	// ========================================================
	// DEBUG
	// ========================================================

	UFUNCTION(BlueprintCallable, Category="MVS|Debug")
	void DebugPrintMVSState() const;
};
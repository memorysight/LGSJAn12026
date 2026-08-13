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


	// --------------------------------------------------------
	// FIRST MVS COMPOSITION
	// --------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MVS|Composition")
	TObjectPtr<USoundBase> BaseComposition = nullptr;


	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> BaseAudioComponent = nullptr;


public:

	// --------------------------------------------------------
	// PLAYBACK
	// --------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category="MVS|Playback")
	void StartMusic();

	UFUNCTION(BlueprintCallable, Category="MVS|Playback")
	void StopMusic();
};
// Copyright Epic Games, Inc. All Rights Reserved.

#include "LGSCoreJan12026GameMode.h"
#include "LGSCoreJan12026Character.h"
#include "UObject/ConstructorHelpers.h"

ALGSCoreJan12026GameMode::ALGSCoreJan12026GameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}

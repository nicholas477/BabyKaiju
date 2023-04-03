// Copyright Epic Games, Inc. All Rights Reserved.

#include "BabyKaijuGameMode.h"
#include "BabyKaijuCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABabyKaijuGameMode::ABabyKaijuGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/BabyKaiju/Blueprints/Player/BP_BabyKaijuPlayer"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

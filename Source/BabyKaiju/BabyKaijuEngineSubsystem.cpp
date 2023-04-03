// Fill out your copyright notice in the Description page of Project Settings.


#include "BabyKaijuEngineSubsystem.h"

#include "AbilitySystemGlobals.h"

void UBabyKaijuEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	UAbilitySystemGlobals::Get().InitGlobalData();
}

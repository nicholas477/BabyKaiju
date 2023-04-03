// Fill out your copyright notice in the Description page of Project Settings.


#include "BabyKaijuBuilding.h"
#include "BabyKaijuAbilitySystemComponent.h"

// Sets default values
ABabyKaijuBuilding::ABabyKaijuBuilding()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UBabyKaijuAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void ABabyKaijuBuilding::BeginPlay()
{
	Super::BeginPlay();
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}


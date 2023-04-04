// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildingHealthComponent.h"

#include "GameFramework/PlayerState.h"

void UBuildingHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OnDie.AddDynamic(this, &UBuildingHealthComponent::GivePointsOnDie);
}

void UBuildingHealthComponent::GivePointsOnDie(UHealthComponent* HealthComponent, AController* Instigator, AActor* DamageCauser)
{
	if (Instigator == nullptr)
	{
		return;
	}

	if (APlayerState* PS = Instigator->GetPlayerState<APlayerState>())
	{
		PS->Score += DestructionScore;
	}
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthKillVolume.h"

#include "HealthComponent.h"

void AHealthKillVolume::ActorEnteredVolume(class AActor* Other)
{
	if (Other)
	{
		if (UHealthComponent* HealthComponent = Cast<UHealthComponent>(Other->GetComponentByClass(UHealthComponent::StaticClass())))
		{
			HealthComponent->Die(nullptr, this);
			return;
		}
	}

	Super::ActorEnteredVolume(Other);
}
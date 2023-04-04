// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HealthComponent.h"
#include "BuildingHealthComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Health), Blueprintable, BlueprintType, meta = (BlueprintSpawnableComponent))
class BABYKAIJU_API UBuildingHealthComponent : public UHealthComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
		float DestructionScore = 100.f;

	UFUNCTION()
		virtual void GivePointsOnDie(UHealthComponent* HealthComponent, AController* Instigator, AActor* DamageCauser);
};

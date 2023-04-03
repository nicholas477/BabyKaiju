// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "BabyKaijuBuilding.generated.h"

UCLASS()
class BABYKAIJU_API ABabyKaijuBuilding : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABabyKaijuBuilding();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return (UAbilitySystemComponent*)AbilitySystemComponent; };

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay Abilities", meta = (AllowPrivateAccess = "true"))
		class UBabyKaijuAbilitySystemComponent* AbilitySystemComponent;

	virtual void BeginPlay() override;
};

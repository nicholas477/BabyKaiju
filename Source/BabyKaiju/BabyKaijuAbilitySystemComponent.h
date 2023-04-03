// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "BabyKaijuAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class BABYKAIJU_API UBabyKaijuAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	// Abilities that are given on component startup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Startup")
		TArray<TSubclassOf<class UGameplayAbility>> StartupAbilities;

	// These effects are only applied one time on startup
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Startup")
		TArray<TSubclassOf<class UGameplayEffect>> StartupEffects;

	/**
	 *	Initialized the Abilities' ActorInfo - the structure that holds information about who we are acting on and who controls us.
	 *      OwnerActor is the actor that logically owns this component.
	 *		AvatarActor is what physical actor in the world we are acting on. Usually a Pawn but it could be a Tower, Building, Turret, etc, may be the same as Owner
	 *
	 *	Also adds startup abilities
	 */
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

protected:
	void AddStartupAbilities();
	bool bHasAddedStartupAbilities = false;
	bool bHasAddedStartupEffects = false;

	void BindAbilityInput(TSubclassOf<class UBabyKaijuGameplayAbility> Ability);
	void OnInputAction(const struct FInputActionValue& Value, TSubclassOf<class UBabyKaijuGameplayAbility> Ability, ETriggerEvent TriggerEvent);

	TMap<TSubclassOf<class UBabyKaijuGameplayAbility>, TArray<TUniquePtr<FEnhancedInputActionEventBinding>>> InputBindings;
};

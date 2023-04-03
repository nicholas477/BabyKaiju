// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "InputTriggers.h"
#include "EnhancedInputComponent.h"
#include "BabyKaijuGameplayAbility.generated.h"

DECLARE_DYNAMIC_DELEGATE_FourParams(FOnAbilityInput, const struct FInputActionValue&, Value, class UInputAction*, BoundInputAction, ETriggerEvent, TriggerEvent, FGameplayAbilityActorInfo, ActorInfo);

/**
 * 
 */
UCLASS()
class BABYKAIJU_API UBabyKaijuGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	/** Input Action that will call this ability */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* InputAction;

	/** What trigger event will activate this ability */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		TSet<ETriggerEvent> InputActionTriggerEvents = { ETriggerEvent::Triggered };

	UFUNCTION(BlueprintNativeEvent, Category = "Input")
		void OnInputActionTriggered(const struct FInputActionValue& Value, ETriggerEvent TriggerEvent, FGameplayAbilityActorInfo ActorInfo) const;

	// Use this to bind/unbind inputs after an ability was activated
	UFUNCTION(BlueprintCallable, Category = "Input")
		void BindAbilityInputAction(const TSet<ETriggerEvent>& TriggerEvents, class UInputAction* BoundInputAction, const FOnAbilityInput& Delegate, bool bRemoveOnAbilityEnd = true);

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	void OnAbilityInputAction(const struct FInputActionValue& Value, ETriggerEvent TriggerEvent, class UInputAction* BoundInputAction, FOnAbilityInput Delegate);

	// Input bindings that are removed on ability end
	TArray<TUniquePtr<FEnhancedInputActionEventBinding>> InputBindings;
};

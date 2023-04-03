// Fill out your copyright notice in the Description page of Project Settings.


#include "BabyKaijuGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"

void UBabyKaijuGameplayAbility::OnInputActionTriggered_Implementation(const struct FInputActionValue& Value, ETriggerEvent TriggerEvent, FGameplayAbilityActorInfo ActorInfo) const
{
	if (TriggerEvent == ETriggerEvent::Completed)
	{
		ActorInfo.AbilitySystemComponent->TryActivateAbilityByClass(GetClass());
	}
}

void UBabyKaijuGameplayAbility::BindAbilityInputAction(const TSet<ETriggerEvent>& TriggerEvents, class UInputAction* BoundInputAction, const FOnAbilityInput& Delegate, bool bRemoveOnAbilityEnd)
{
	check(BoundInputAction);
	check(Delegate.IsBound());

	if (GetAbilitySystemComponentFromActorInfo() == nullptr)
	{
		return;
	}

	if (APawn* PawnOwner = Cast<APawn>(GetAvatarActorFromActorInfo()))
	{
		if (UEnhancedInputComponent* InputComponent = Cast<UEnhancedInputComponent>(PawnOwner->InputComponent))
		{
			for (ETriggerEvent TriggerEvent : TriggerEvents)
			{
				if (TriggerEvent != ETriggerEvent::None)
				{
					FEnhancedInputActionEventBinding& InputBinding = InputComponent->BindAction(BoundInputAction, TriggerEvent, this, &UBabyKaijuGameplayAbility::OnAbilityInputAction, TriggerEvent, BoundInputAction, Delegate);

					if (bRemoveOnAbilityEnd)
					{
						InputBindings.Emplace(InputBinding.Clone());
					}
				}
			}
		}
	}
}

void UBabyKaijuGameplayAbility::OnAbilityInputAction(const struct FInputActionValue& Value, ETriggerEvent TriggerEvent, class UInputAction* BoundInputAction, FOnAbilityInput Delegate)
{
	if (!Delegate.IsBound())
	{
		return;
	}

	if (GetAbilitySystemComponentFromActorInfo() == nullptr)
	{
		return;
	}

	Delegate.Execute(Value, BoundInputAction, TriggerEvent, *GetAbilitySystemComponentFromActorInfo()->AbilityActorInfo.Get());
}

void UBabyKaijuGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if (APawn* PawnOwner = Cast<APawn>(ActorInfo->AvatarActor))
		{
			if (UEnhancedInputComponent* InputComponent = Cast<UEnhancedInputComponent>(PawnOwner->InputComponent))
			{
				for (TUniquePtr<FEnhancedInputActionEventBinding>& InputBinding : InputBindings)
				{
					InputComponent->RemoveBinding(*InputBinding);
				}
				InputBindings.Empty();
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "BabyKaijuAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayTagResponseTable.h"
#include "BabyKaijuGameplayAbility.h"
#include "EnhancedInputComponent.h"

void UBabyKaijuAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	check(AbilityActorInfo.IsValid());
	bool WasAbilityActorNull = (AbilityActorInfo->AvatarActor == nullptr);
	bool AvatarChanged = (InAvatarActor != AbilityActorInfo->AvatarActor);

	AbilityActorInfo->InitFromActor(InOwnerActor, InAvatarActor, this);

	SetOwnerActor(InOwnerActor);

	// caching the previous value of the actor so we can check against it but then setting the value to the new because it may get used
	const AActor* PrevAvatarActor = GetAvatarActor_Direct();
	SetAvatarActor_Direct(InAvatarActor);

	AddStartupAbilities();

	// if the avatar actor was null but won't be after this, we want to run the deferred gameplaycues that may not have run in NetDeltaSerialize
	// Conversely, if the ability actor was previously null, then the effects would not run in the NetDeltaSerialize. As such we want to run them now.
	if ((WasAbilityActorNull || PrevAvatarActor == nullptr) && InAvatarActor != nullptr)
	{
		HandleDeferredGameplayCues(&ActiveGameplayEffects);
	}

	if (AvatarChanged)
	{
		ABILITYLIST_SCOPE_LOCK();
		for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
		{
			if (Spec.Ability)
			{
				Spec.Ability->OnAvatarSet(AbilityActorInfo.Get(), Spec);
			}
		}
	}

	if (UGameplayTagReponseTable* TagTable = UAbilitySystemGlobals::Get().GetGameplayTagResponseTable())
	{
		TagTable->RegisterResponseForEvents(this);
	}

	LocalAnimMontageInfo = FGameplayAbilityLocalAnimMontage();
	if (IsOwnerActorAuthoritative())
	{
		SetRepAnimMontageInfo(FGameplayAbilityRepAnimMontage());
	}

	if (bPendingMontageRep)
	{
		OnRep_ReplicatedAnimMontage();
	}
}

void UBabyKaijuAbilitySystemComponent::BindInputs()
{
	for (const TSubclassOf<UGameplayAbility>& StartupAbility : StartupAbilities)
	{
		if (StartupAbility.Get())
		{
			if (StartupAbility->IsChildOf<UBabyKaijuGameplayAbility>())
			{
				BindAbilityInput(*StartupAbility);
			}
		}
	}
}

void UBabyKaijuAbilitySystemComponent::AddStartupAbilities()
{
	if (!bHasAddedStartupAbilities)
	{
		if (IsOwnerActorAuthoritative())
		{
			for (const TSubclassOf<UGameplayAbility>& StartupAbility : StartupAbilities)
			{
				if (StartupAbility.Get())
				{
					FGameplayAbilitySpecHandle Handle;
					Handle = GiveAbility(FGameplayAbilitySpec(StartupAbility, 1, -1, this));
				}
			}
		}

		bHasAddedStartupAbilities = true;
	}

	BindInputs();

	if (!bHasAddedStartupEffects && GetAttributeSet(UAttributeSet::StaticClass()))
	{
		if (IsOwnerActorAuthoritative())
		{
			FGameplayEffectContextHandle EffectContext = MakeEffectContext();
			EffectContext.AddSourceObject(GetTypedOuter<AActor>());

			for (TSubclassOf<UGameplayEffect> GameplayEffect : StartupEffects)
			{
				FGameplayEffectSpecHandle NewHandle = MakeOutgoingSpec(GameplayEffect, 1.0, EffectContext);
				if (NewHandle.IsValid())
				{
					FActiveGameplayEffectHandle ActiveGEHandle = ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
				}
			}
		}

		bHasAddedStartupEffects = true;
	}
}

void UBabyKaijuAbilitySystemComponent::BindAbilityInput(TSubclassOf<UBabyKaijuGameplayAbility> Ability)
{
	check(Ability);

	if (InputBindings.Contains(Ability))
	{
		return;
	}

	UInputAction* InputAction = Ability->GetDefaultObject<UBabyKaijuGameplayAbility>()->InputAction;
	const TSet<ETriggerEvent>& TriggerEvents = Ability->GetDefaultObject<UBabyKaijuGameplayAbility>()->InputActionTriggerEvents;

	APawn* PawnOwner = Cast<APawn>(GetOwner());

	if (PawnOwner && InputAction)
	{
		if (UEnhancedInputComponent* InputComponent = Cast<UEnhancedInputComponent>(PawnOwner->InputComponent))
		{
			for (ETriggerEvent TriggerEvent : TriggerEvents)
			{
				if (TriggerEvent != ETriggerEvent::None)
				{
					FEnhancedInputActionEventBinding& InputBinding = InputComponent->BindAction(InputAction, TriggerEvent, this, &UBabyKaijuAbilitySystemComponent::OnInputAction, Ability, TriggerEvent);
					TArray<TUniquePtr<FEnhancedInputActionEventBinding>>& EventBindings = InputBindings.FindOrAdd(Ability);
					EventBindings.Add(InputBinding.Clone());
				}
			}
		}
	}
}

void UBabyKaijuAbilitySystemComponent::OnInputAction(const FInputActionValue& Value, TSubclassOf<class UBabyKaijuGameplayAbility> Ability, ETriggerEvent TriggerEvent)
{
	Ability->GetDefaultObject<UBabyKaijuGameplayAbility>()->OnInputActionTriggered(Value, TriggerEvent, *AbilityActorInfo.Get());
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "AbilitySystemGlobals.h"

UHealthAttributeSet::UHealthAttributeSet()
{

}

void UHealthAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	// This is called whenever attributes change, so for max health/mana we want to scale the current totals to match
	Super::PreAttributeChange(Attribute, NewValue);

	// If a Max value changes, adjust current to keep Current % of Current to Max
	if (Attribute == GetMaxHealthAttribute()) // GetMaxHealthAttribute comes from the Macros defined at the top of the header
	{
		AdjustAttributeForMaxChange(Health, MaxHealth, NewValue, GetHealthAttribute());
	}
}

void UHealthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

void UHealthAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UHealthAttributeSet::Serialize(FArchive& Ar)
{
	if (Ar.IsSaving() && Ar.ArIsSaveGame)
	{
		SerializedHealth = Health.GetCurrentValue();
		SerializedMaxHealth = MaxHealth.GetCurrentValue();
	}

	Super::Serialize(Ar);

	if (Ar.IsLoading() && Ar.ArIsSaveGame)
	{
		SetHealth(SerializedHealth);
		SetMaxHealth(SerializedMaxHealth);
	}
}

UAbilitySystemComponent* UHealthAttributeSet::GetOwningAbilitySystemComponent() const
{
	return GetTypedOuter<UHealthComponent>()->GetAbilitySystemComponent();
}

UAbilitySystemComponent* UHealthAttributeSet::GetOwningAbilitySystemComponentChecked() const
{
	UAbilitySystemComponent* Result = GetOwningAbilitySystemComponent();
	check(Result);
	return Result;
}

void UHealthAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, Health, OldHealth);
}

void UHealthAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, MaxHealth, OldMaxHealth);
}

void UHealthAttributeSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
{
	UAbilitySystemComponent* AbilityComp = GetTypedOuter<UHealthComponent>()->GetAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilityComp)
	{
		// Change current value to maintain the current Val / Max percent
		const float CurrentValue = AffectedAttribute.GetCurrentValue();
		float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

		AbilityComp->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}

UHealthComponent::UHealthComponent()
{
	InitialMaxHealth = 100.f;
	bTakeFallDamage = true;
	bGodMode = false;

	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;

	InputSpeedRange = FFloatRange(2000.f, 8000.f);
	OutputDamageRange = FFloatRange(0.f, 100.f);

	AttributeSet = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("AttributeSet"));
	AttributeSet->InitMaxHealth(InitialMaxHealth);

	bApplyDamageOnTakeRadialDamage = true;
	bApplyDamageOnTakePointDamage = true;
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, bGodMode);
}

void UHealthComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (GetOwner()->HasAuthority())
	{
		AttributeSet->InitHealth(InitialMaxHealth);
		AttributeSet->InitMaxHealth(InitialMaxHealth);
		OnTakeDamage.Broadcast(this, 0, InitialMaxHealth);
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			ASC->AddSpawnedAttribute(AttributeSet);
			ASC->ForceReplication();
		}
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxHealthAttribute()).AddUObject(this, &UHealthComponent::OnMaxHealthChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &UHealthComponent::OnHealthChanged);
	}
}

UAbilitySystemComponent* UHealthComponent::GetAbilitySystemComponent() const
{
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetTypedOuter<AActor>());
}

void UHealthComponent::Damage(float Amount, AController* Instigator, AActor* DamageCauser)
{
	if (GetOwner()->CanBeDamaged() && !bGodMode)
	{
		const float OldHealth = AttributeSet->Health.GetCurrentValue();
		const float NewHealth = AttributeSet->Health.GetCurrentValue() - Amount;

		OnTakeDamage.Broadcast(this, OldHealth, NewHealth);
		if (OldHealth > 0.f && NewHealth <= 0.f)
		{
			Die(Instigator, DamageCauser);
		}
		else
		{
			SetHealth(NewHealth);
		}
	}
}

void UHealthComponent::Heal(float Amount)
{
	const float OldHealth = AttributeSet->Health.GetCurrentValue();
	const float NewHealth = FMath::Clamp(AttributeSet->Health.GetCurrentValue() + Amount, 0.f, AttributeSet->MaxHealth.GetCurrentValue());
	SetHealth(NewHealth);

	OnTakeDamage.Broadcast(this, OldHealth, AttributeSet->Health.GetCurrentValue());
}

void UHealthComponent::SetHealth(float NewHealth)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		GetAbilitySystemComponent()->ApplyModToAttributeUnsafe(AttributeSet->GetHealthAttribute(), EGameplayModOp::Override, NewHealth);
		return;
	}
	else
	{
		AttributeSet->Health.SetCurrentValue(NewHealth);
	}
}

void UHealthComponent::SetMaxHealth(float NewMaxHealth)
{
	if (NewMaxHealth == GetMaxHealth())
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		GetAbilitySystemComponent()->ApplyModToAttributeUnsafe(AttributeSet->GetMaxHealthAttribute(), EGameplayModOp::Override, NewMaxHealth);
	}
	else
	{
		AttributeSet->MaxHealth.SetCurrentValue(NewMaxHealth);
	}
}

void UHealthComponent::Die(AController* Instigator, AActor* DamageCauser)
{
	if (!IsDead() && !bGodMode)
	{
		SetHealth(FMath::Min(AttributeSet->Health.GetCurrentValue(), 0.f));

		// Apply the dead effect
		if (DeadEffect != nullptr)
		{
			if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
			{
				if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
				{
					auto EffectContext = ASC->MakeEffectContext();
					DeadEffectHandle = ASC->ApplyGameplayEffectToSelf(DeadEffect->GetDefaultObject<UGameplayEffect>(), 1.f, EffectContext);
				}
			}
		}

		Multicast_Die(Instigator, DamageCauser);
	}
}

void UHealthComponent::Resurrect()
{
	if (IsDead())
	{
		// Remove the dead effect
		if (DeadEffectHandle.IsValid())
		{
			if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
			{
				if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
				{
					DeadEffectHandle.GetOwningAbilitySystemComponent()->RemoveActiveGameplayEffect(DeadEffectHandle);
					DeadEffectHandle = FActiveGameplayEffectHandle();
				}
			}
		}

		SetHealth(AttributeSet->MaxHealth.GetCurrentValue());
		Multicast_Resurrect();
	}
}

void UHealthComponent::Multicast_Die_Implementation(AController* Instigator, AActor* DamageCauser)
{
	K2_OnDie(Instigator, DamageCauser);
	OnDie.Broadcast(this, Instigator, DamageCauser);
}

void UHealthComponent::Multicast_Resurrect_Implementation()
{
	OnResurrect.Broadcast(this);
}

bool UHealthComponent::IsDead() const
{
	return AttributeSet->Health.GetCurrentValue() <= 0.f;
}

float UHealthComponent::GetHealth() const
{
	return AttributeSet->Health.GetCurrentValue();
}

float UHealthComponent::GetMaxHealth() const
{
	return AttributeSet->MaxHealth.GetCurrentValue();
}

void UHealthComponent::Serialize(FArchive& Ar)
{
	float OldHealth = AttributeSet->Health.GetCurrentValue();

	if (Ar.ArIsSaveGame && Ar.IsSaving())
	{
		FMemoryWriter MemWriter(AttributeSetSerializedData);
		FObjectAndNameAsStringProxyArchive AttributeSetAr(MemWriter, true);
		AttributeSetAr.ArIsSaveGame = true;
		AttributeSet->Serialize(AttributeSetAr);
	}

	Super::Serialize(Ar);

	if (Ar.ArIsSaveGame && Ar.IsLoading())
	{
		FMemoryReader MemReader(AttributeSetSerializedData);
		FObjectAndNameAsStringProxyArchive AttributeSetAr(MemReader, true);
		AttributeSetAr.ArIsSaveGame = true;
		AttributeSet->Serialize(AttributeSetAr);

		OnHealthDeserialize.Broadcast(this);

		if (OldHealth != 0.f && AttributeSet->Health.GetCurrentValue() <= 0.f)
		{
			// Apply the dead effect
			if (DeadEffect != nullptr)
			{
				if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
				{
					if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
					{
						auto EffectContext = ASC->MakeEffectContext();
						DeadEffectHandle = ASC->ApplyGameplayEffectToSelf(DeadEffect->GetDefaultObject<UGameplayEffect>(), 1.f, EffectContext);
					}
				}
			}
			K2_OnDead();
			OnDead.Broadcast(this);
		}
	}
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (!GetOwner()->OnTakeRadialDamage.Contains(this, FName("OnTakeRadialDamage")))
		{
			GetOwner()->OnTakeRadialDamage.AddDynamic(this, &UHealthComponent::OnTakeRadialDamage);
		}

		if (!GetOwner()->OnTakePointDamage.Contains(this, FName("OnTakePointDamage")))
		{
			GetOwner()->OnTakePointDamage.AddDynamic(this, &UHealthComponent::OnTakePointDamage);
		}

		if (ACharacter* CharacterOwner = GetOwner<ACharacter>())
		{
			if (!CharacterOwner->LandedDelegate.Contains(this, FName("OnOwnerLanded")))
			{
				CharacterOwner->LandedDelegate.AddDynamic(this, &UHealthComponent::OnOwnerLanded);
			}
		}
	}
}

void UHealthComponent::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	OnMaxHealthChange.Broadcast(this);
}

void UHealthComponent::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	CurrentHealth = Data.NewValue;
	OnTakeDamage.Broadcast(this, Data.OldValue, Data.NewValue);
}

void UHealthComponent::OnTakeRadialDamage_Implementation(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, FVector Origin, FHitResult HitInfo, class AController* InstigatedBy, AActor* DamageCauser)
{
	OnTakeRadialDamageDelegate.Broadcast(DamagedActor, Damage, DamageType, Origin, HitInfo, InstigatedBy, DamageCauser);

	if (bApplyDamageOnTakeRadialDamage)
	{
		this->Damage(Damage, InstigatedBy, DamageCauser);
	}
}

void UHealthComponent::OnTakePointDamage_Implementation(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* HitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	OnTakePointDamageDelegate.Broadcast(DamagedActor, Damage, InstigatedBy, HitLocation, HitComponent, BoneName, ShotFromDirection, DamageType, DamageCauser);

	if (bApplyDamageOnTakePointDamage)
	{
		this->Damage(Damage, InstigatedBy, DamageCauser);
	}
}

void UHealthComponent::OnOwnerLanded_Implementation(const FHitResult& Hit)
{
	if (bTakeFallDamage)
	{
		float FallingSpeed = FVector::DotProduct(GetOwner<ACharacter>()->GetVelocity(), FVector(0.f, 0.f, -1.f));
		if (FallingSpeed >= InputSpeedRange.GetLowerBoundValue())
		{
			float Damage = FMath::GetMappedRangeValueClamped(InputSpeedRange, OutputDamageRange, FallingSpeed);

			this->Damage(Damage, nullptr, nullptr);
		}
	}
}

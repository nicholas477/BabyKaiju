// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Math/Range.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "HealthComponent.generated.h"

class UHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthComponentTakeDamage, UHealthComponent*, HealthComponent, float, OldHealth, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthComponentDie, UHealthComponent*, HealthComponent, AController*, Instigator, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthComponentDead, UHealthComponent*, HealthComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthComponentResurrect, UHealthComponent*, HealthComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthDeserialize, UHealthComponent*, HealthComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthChange, UHealthComponent*, HealthComponent);

class UHealthAttributeSet;

UCLASS( ClassGroup=(Custom), Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent) )
class HEALTH_API UHealthComponent : public UActorComponent, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	virtual void InitializeComponent() override;

	UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "Health")
		void Damage(float Amount, AController* Instigator, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "Health")
		void Heal(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Health")
		void SetHealth(float NewHealth);

	UFUNCTION(BlueprintCallable, Category = "Health")
		void SetMaxHealth(float NewMaxHealth);

	UFUNCTION(BlueprintCallable, Category = "Health")
		void Die(AController* Instigator, AActor* DamageCauser);

	UFUNCTION(BlueprintImplementableEvent, Category = "Health", meta=(DisplayName="On Die"))
		void K2_OnDie(AController* Instigator, AActor* DamageCauser);

	UFUNCTION(BlueprintImplementableEvent, Category = "Health", meta = (DisplayName = "On Dead"))
		void K2_OnDead();

	UFUNCTION(BlueprintCallable, Category = "Health")
		void Resurrect();

	UPROPERTY(BlueprintAssignable, Category = "Health")
		FOnHealthComponentTakeDamage OnTakeDamage;

	UPROPERTY(BlueprintAssignable, Category = "Health")
		FOnMaxHealthChange OnMaxHealthChange;

	UPROPERTY(BlueprintAssignable, Category = "Health")
		FOnHealthComponentDie OnDie;

	UPROPERTY(BlueprintAssignable, Category = "Health")
		FOnHealthDeserialize OnHealthDeserialize;

	UPROPERTY(BlueprintAssignable, Category = "Health")
		FOnHealthComponentDead OnDead;

	UPROPERTY(BlueprintAssignable, Category = "Health")
		FOnHealthComponentResurrect OnResurrect;

	UFUNCTION(BlueprintPure, Category = "Health")
		bool IsDead() const;

	UFUNCTION(BlueprintPure, Category = "Health")
		float GetHealth() const;

	UFUNCTION(BlueprintPure, Category = "Health")
		float GetMaxHealth() const;

	virtual void Serialize(FArchive& Ar) override;

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Health", meta = (AllowPrivateAccess = "true"))
		float InitialMaxHealth;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Category = "Health", meta = (AllowPrivateAccess = "true"))
		bool bGodMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
		UHealthAttributeSet* AttributeSet;

	// This effect will be added when the player is dead
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|GAS", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<class UGameplayEffect> DeadEffect;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Health|GAS")
		FActiveGameplayEffectHandle DeadEffectHandle;

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_Die(AController* Instigator, AActor* DamageCauser);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_Resurrect();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Fall Damage", meta = (AllowPrivateAccess = "true"))
		bool bTakeFallDamage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Fall Damage", meta = (AllowPrivateAccess = "true", EditCondition="bTakeFallDamage"))
		FFloatRange InputSpeedRange;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Fall Damage", meta = (AllowPrivateAccess = "true", EditCondition="bTakeFallDamage"))
		FFloatRange OutputDamageRange;

	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent, Category = "Health")
		void OnTakeRadialDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, FVector Origin, FHitResult HitInfo, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(BlueprintAssignable, Category = "Health")
		FTakeRadialDamageSignature OnTakeRadialDamageDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
		bool bApplyDamageOnTakeRadialDamage;

	UFUNCTION(BlueprintNativeEvent, Category = "Health")
		void OnTakePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);

	UPROPERTY(BlueprintAssignable, Category = "Health")
		FTakePointDamageSignature OnTakePointDamageDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
		bool bApplyDamageOnTakePointDamage;

	// Called when the owner character lands. Used for applying fall damage
	UFUNCTION(BlueprintNativeEvent, Category = "Health")
		void OnOwnerLanded(const FHitResult& Hit);

	void OnMaxHealthChanged(const struct FOnAttributeChangeData& Data);

	UPROPERTY(SaveGame)
		TArray<uint8> AttributeSetSerializedData;

	UPROPERTY(VisibleAnywhere, Category = "Health")
		float CurrentHealth;

	void OnHealthChanged(const struct FOnAttributeChangeData& Data);
};

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 *
 */
UCLASS()
class HEALTH_API UHealthAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UHealthAttributeSet();

	// AttributeSet Overrides
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_Health)
		FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_MaxHealth)
		FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, MaxHealth)

	virtual void Serialize(FArchive& Ar) override;

	/** Gets information about owning actor */
	FORCEINLINE AActor* GetOwningActor() const { return CastChecked<AActor>(GetTypedOuter<UHealthComponent>()->GetOuter()); }
	UAbilitySystemComponent* GetOwningAbilitySystemComponent() const;
	UAbilitySystemComponent* GetOwningAbilitySystemComponentChecked() const;

protected:
	UFUNCTION()
		virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
		virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	// Helper function to proportionally adjust the value of an attribute when it's associated max attribute changes.
	// (i.e. When MaxHealth increases, Health increases by an amount that maintains the same percentage as before)
	void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);

	UPROPERTY(SaveGame)
		float SerializedHealth;

	UPROPERTY(SaveGame)
		float SerializedMaxHealth;
};

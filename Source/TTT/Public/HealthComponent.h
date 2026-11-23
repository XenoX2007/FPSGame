// Reusable health for any actor (player and enemies): one set of rules, tested once.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHealthChangedDelegate, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHealthDepletedDelegate, AActor*, DeadActor);

/**
 * Holds an actor's health and tells listeners when it changes or runs out.
 * The owner decides what dying means (the component only reports it).
 */
UCLASS(ClassGroup = (TTT), meta = (BlueprintSpawnableComponent))
class TTT_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	/** Health the owner starts with. The owner dies when health reaches 0. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (ClampMin = 1))
	float MaxHealth = 100.0f;

	/** Fired whenever health changes (and once at BeginPlay). Bind this to update a health bar. */
	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FHealthChangedDelegate OnHealthChanged;

	/** Fired once when health reaches 0. */
	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FHealthDepletedDelegate OnHealthDepleted;

	// Removes up to Amount health. Returns the damage actually taken (0 if dead or Amount <= 0).
	float ApplyDamage(float Amount);

	// Restores health, clamped to MaxHealth. Returns false if nothing changed (dead, full or Amount <= 0).
	bool Heal(float Amount);

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }

	/** 0..1, handy for progress bars. */
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const { return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return CurrentHealth <= 0.0f; }

	bool IsFull() const { return CurrentHealth >= MaxHealth; }

protected:
	virtual void BeginPlay() override;

private:
	/** Health left right now (runtime only). */
	UPROPERTY(VisibleInstanceOnly, Category = "Health")
	float CurrentHealth = 0.0f;
};

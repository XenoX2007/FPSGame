// Reusable health for any actor (player and enemies): one set of rules, tested once.
#include "HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	// Health only changes on damage/heal events, never per frame
	PrimaryComponentTick.bCanEverTick = false;

	// Sensible value before BeginPlay; BeginPlay re-reads MaxHealth once Blueprint defaults are applied
	CurrentHealth = MaxHealth;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Blueprint defaults are applied after the constructor, so read MaxHealth here
	CurrentHealth = MaxHealth;
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

float UHealthComponent::ApplyDamage(float Amount)
{
	if (Amount <= 0.0f || IsDead())
	{
		return 0.0f;
	}

	// Never take more than what is left, so health stops at exactly 0
	const float Taken = FMath::Min(Amount, CurrentHealth);
	CurrentHealth -= Taken;
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (IsDead())
	{
		// Only reachable once: the IsDead() guard above blocks any later damage
		OnHealthDepleted.Broadcast(GetOwner());
	}
	return Taken;
}

bool UHealthComponent::Heal(float Amount)
{
	// Can't heal the dead, and nothing to do at full health
	if (Amount <= 0.0f || IsDead() || IsFull())
	{
		return false;
	}

	CurrentHealth = FMath::Min(CurrentHealth + Amount, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	return true;
}

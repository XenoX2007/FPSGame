// Pickup that restores the player's HP.
#include "HealthPickup.h"
#include "ShooterCharacter.h"
#include "HealthComponent.h"

bool AHealthPickup::ApplyPickup(AActor* Other)
{
	// Player only: enemies walking over the pickup must not use it up
	if (!Cast<AShooterCharacter>(Other))
	{
		return false;
	}

	// Talk to the component, not the character: health rules live in one place
	UHealthComponent* Health = Other->FindComponentByClass<UHealthComponent>();
	return Health && Health->Heal(HealAmount);
}

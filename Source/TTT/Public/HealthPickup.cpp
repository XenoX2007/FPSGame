// Pickup that restores the player's HP.

#include "HealthPickup.h"
#include "ShooterCharacter.h"

bool AHealthPickup::ApplyPickup(AActor* Other)
{
	// Health is not a component yet, so talk to the shooter character directly
	AShooterCharacter* Character = Cast<AShooterCharacter>(Other);
	return Character && Character->Heal(HealAmount);
}

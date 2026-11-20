// Win-condition item: the player must collect all of these before the timer runs out.

#include "CollectiblePickup.h"
#include "ShooterCharacter.h"

ACollectiblePickup::ACollectiblePickup()
{
	// Collected once per round
	bRespawns = false;
}

bool ACollectiblePickup::ApplyPickup(AActor* Other)
{
	// Only the player counts (enemies are not AShooterCharacter)
	if (!Cast<AShooterCharacter>(Other))
	{
		return false;
	}

	// Observer: GameMode counts it, Blueprint children can play feedback
	OnCollected.Broadcast(this);
	return true;
}

// Pickup that adds armour to any actor with a UShooterArmourComponent.

#include "ShieldPickup.h"
#include "ShooterArmourComponent.h"

bool AShieldPickup::ApplyPickup(AActor* Other)
{
	// Works on any actor that has an armour component - no cast to a specific character class
	UShooterArmourComponent* Armour = Other->FindComponentByClass<UShooterArmourComponent>();
	if (!Armour || Armour->IsArmourFull())
	{
		return false;
	}

	Armour->AddArmour(ArmourAmount);
	return true;
}

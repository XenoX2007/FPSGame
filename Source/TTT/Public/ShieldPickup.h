// Pickup that adds armour to any actor with a UShooterArmourComponent.

#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "ShieldPickup.generated.h"

UCLASS()
class TTT_API AShieldPickup : public APickupBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="Pickup")
	float GetArmourAmount() const { return ArmourAmount; }

protected:
	/** Armour added on pickup (clamped to the component's MaxArmour) */
	UPROPERTY(EditAnywhere, Category="Pickup", meta = (ClampMin = 0))
	float ArmourAmount = 25.0f;

	virtual bool ApplyPickup(AActor* Other) override;
};

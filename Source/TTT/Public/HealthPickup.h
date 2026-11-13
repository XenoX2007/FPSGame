// Pickup that restores the player's HP.

#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "HealthPickup.generated.h"

UCLASS()
class TTT_API AHealthPickup : public APickupBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="Pickup")
	float GetHealAmount() const { return HealAmount; }

protected:
	/** HP restored on pickup (clamped to the character's MaxHP) */
	UPROPERTY(EditAnywhere, Category="Pickup", meta = (ClampMin = 0))
	float HealAmount = 25.0f;

	virtual bool ApplyPickup(AActor* Other) override;
};

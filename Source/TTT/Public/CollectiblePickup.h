// Win-condition item: the player must collect all of these before the timer runs out.

#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "CollectiblePickup.generated.h"

class ACollectiblePickup;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollectedDelegate, ACollectiblePickup*, Pickup);

UCLASS()
class TTT_API ACollectiblePickup : public APickupBase
{
	GENERATED_BODY()

public:
	ACollectiblePickup();

	/** Broadcast when the player collects this item (GameMode counts it, Blueprint plays VFX/SFX) */
	UPROPERTY(BlueprintAssignable, Category="Pickup|Events")
	FOnCollectedDelegate OnCollected;

protected:
	virtual bool ApplyPickup(AActor* Other) override;
};

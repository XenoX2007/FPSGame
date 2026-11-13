// Shared base for collectible pickups (health, shield, ...).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;

/**
 * Handles everything pickups have in common: overlap sphere, mesh, consume -> hide -> respawn.
 * Child classes only decide what the pickup does by overriding ApplyPickup().
 */
UCLASS(Abstract)
class TTT_API APickupBase : public AActor
{
	GENERATED_BODY()

	/** Overlap sphere that detects the player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> SphereCollision;

	/** Visual mesh, set in the Blueprint child */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Mesh;

public:
	APickupBase();

	/** Applies the pickup to Other and consumes it. Returns false (and stays available) if it had no effect. */
	UFUNCTION(BlueprintCallable, Category="Pickup")
	bool TryPickup(AActor* Other);

	UFUNCTION(BlueprintPure, Category="Pickup")
	bool IsAvailable() const { return bIsAvailable; }

	UFUNCTION(BlueprintPure, Category="Pickup")
	float GetRespawnTime() const { return RespawnTime; }

protected:
	/** Time to wait before respawning this pickup */
	UPROPERTY(EditAnywhere, Category="Pickup", meta = (ClampMin = 0, ClampMax = 120, Units = "s"))
	float RespawnTime = 4.0f;

	/** What the pickup does. Return true only if it actually had an effect (so it gets consumed). */
	virtual bool ApplyPickup(AActor* Other) { return false; }

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Called when RespawnTime has passed */
	void RespawnPickup();

	/** Blueprint can override to animate the respawn; it must end by calling FinishRespawn.
	 *  The C++ default calls FinishRespawn straight away. */
	UFUNCTION(BlueprintNativeEvent, Category="Pickup", meta = (DisplayName = "OnRespawn"))
	void BP_OnRespawn();

	/** Makes the pickup collectible again */
	UFUNCTION(BlueprintCallable, Category="Pickup")
	void FinishRespawn();

private:
	bool bIsAvailable = true;
	FTimerHandle RespawnTimer;
};



#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "HealthPickup.generated.h"


class USphereComponent;
class UPrimitiveComponent;
UCLASS()


class TTT_API AHealthPickup : public AActor
{
	GENERATED_BODY()
	
	/** Collision sphere */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* SphereCollision;

	/** Health pickup mesh. Its mesh asset is set from the weapon data table */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;

public:
	// Sets default values for this actor's properties
	AHealthPickup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere, Category="Pickup")
	float ArmourAmount = 25.0f;
	/** Time to wait before respawning this pickup */
	UPROPERTY(EditAnywhere, Category="Pickup", meta = (ClampMin = 0, ClampMax = 120, Units = "s"))
	float RespawnTime = 4.0f;

	/** Timer to respawn the pickup */
	FTimerHandle RespawnTimer;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
protected:

	/** Native construction script */
	virtual void OnConstruction(const FTransform& Transform) override;

	
	/** Gameplay cleanup */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Handles collision overlap */
	UFUNCTION()
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
protected:

	/** Called when it's time to respawn this pickup */
	void RespawnPickup();

	/** Passes control to Blueprint to animate the pickup respawn. Should end by calling FinishRespawn */
	UFUNCTION(BlueprintImplementableEvent, Category="Pickup", meta = (DisplayName = "OnRespawn"))
	void BP_OnRespawn();

	/** Enables this pickup after respawning */
	UFUNCTION(BlueprintCallable, Category="Pickup")
	void FinishRespawn();

};

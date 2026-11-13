// Shared base for collectible pickups (health, shield, ...).

#include "PickupBase.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

APickupBase::APickupBase()
{
	PrimaryActorTick.bCanEverTick = false; // event-driven, no Tick needed

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Same component names as the old AHealthPickup so existing Blueprint settings are kept
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collision"));
	SphereCollision->SetupAttachment(RootComponent);
	SphereCollision->SetRelativeLocation(FVector(0.0f, 0.0f, 84.0f));
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionObjectType(ECC_WorldStatic);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereCollision->bFillCollisionUnderneathForNavmesh = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SphereCollision);
	Mesh->SetCollisionProfileName(FName("NoCollision"));
}

void APickupBase::BeginPlay()
{
	Super::BeginPlay();
	SphereCollision->OnComponentBeginOverlap.AddUniqueDynamic(this, &APickupBase::OnOverlap);
}

void APickupBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void APickupBase::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TryPickup(OtherActor);
}

bool APickupBase::TryPickup(AActor* Other)
{
	// Only consume the pickup if it actually did something (e.g. not at full HP)
	if (!bIsAvailable || !Other || !ApplyPickup(Other))
	{
		return false;
	}

	// Consume: hide, stop overlapping, schedule the respawn
	bIsAvailable = false;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &APickupBase::RespawnPickup, RespawnTime, false);
	return true;
}

void APickupBase::RespawnPickup()
{
	SetActorHiddenInGame(false);

	// Blueprint may override OnRespawn to animate; the default just finishes immediately
	BP_OnRespawn();
}

void APickupBase::BP_OnRespawn_Implementation()
{
	FinishRespawn();
}

void APickupBase::FinishRespawn()
{
	SetActorEnableCollision(true);
	bIsAvailable = true;
}

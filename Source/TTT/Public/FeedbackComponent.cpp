#include "FeedbackComponent.h"

#include "HealthComponent.h"
#include "TTTCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"

UFeedbackComponent::UFeedbackComponent()
{
	// Purely event driven
	PrimaryComponentTick.bCanEverTick = false;
}

void UFeedbackComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Observer: subscribe to the owner's events; the owner does not know this component exists
	if (UHealthComponent* Health = Owner->FindComponentByClass<UHealthComponent>())
	{
		// Start from max so the health component's own BeginPlay broadcast (full health) is not a "hit"
		LastKnownHealth = Health->GetMaxHealth();
		Health->OnHealthChanged.AddUniqueDynamic(this, &UFeedbackComponent::HandleHealthChanged);
	}

	if (ATTTCharacter* Character = Cast<ATTTCharacter>(Owner))
	{
		Character->OnDashed.AddUniqueDynamic(this, &UFeedbackComponent::HandleDashed);
	}
}

void UFeedbackComponent::PlayFeedback(EFeedbackEvent Event)
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const FFeedbackCue& Cue = GetCue(Event);
	const FVector Location = Owner->GetActorLocation();

	// Fire-and-forget at a location: the cue keeps playing even if the owner is destroyed (enemy death)
	if (Cue.Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Cue.Sound, Location);
	}
	if (Cue.Effect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, Cue.Effect, Location, Owner->GetActorRotation());
	}

	OnFeedbackPlayed.Broadcast(Event);
}

const FFeedbackCue& UFeedbackComponent::GetCue(EFeedbackEvent Event) const
{
	switch (Event)
	{
	case EFeedbackEvent::Death:	return DeathCue;
	case EFeedbackEvent::Dash:	return DashCue;
	default:					return HitCue;
	}
}

void UFeedbackComponent::HandleHealthChanged(float CurrentHealth, float MaxHealth)
{
	// Healing (or the BeginPlay reset) is not a hit
	const bool bTookDamage = CurrentHealth < LastKnownHealth;
	LastKnownHealth = CurrentHealth;
	if (!bTookDamage)
	{
		return;
	}

	// Decide Hit vs Death here, not in OnHealthDepleted: OnHealthChanged fires first, before the owner
	// reacts to death. An enemy destroys itself in its OnHealthDepleted handler, which would stop
	// this component's handler from running if it were bound there.
	PlayFeedback(CurrentHealth > 0.0f ? EFeedbackEvent::Hit : EFeedbackEvent::Death);
}

void UFeedbackComponent::HandleDashed()
{
	PlayFeedback(EFeedbackEvent::Dash);
}

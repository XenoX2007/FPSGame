// Sound + special effects for any actor: listens to the owner's events and plays the matching cue.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FeedbackComponent.generated.h"

class USoundBase;
class UNiagaraSystem;

/** Gameplay moments that have an audio-visual cue. */
UENUM(BlueprintType)
enum class EFeedbackEvent : uint8
{
	Hit		UMETA(ToolTip = "Owner took damage but is still alive."),
	Death	UMETA(ToolTip = "Owner's health reached zero."),
	Dash	UMETA(ToolTip = "Owner performed a dash.")
};

/** One cue: a sound and/or a Niagara effect, both optional. */
USTRUCT(BlueprintType)
struct FFeedbackCue
{
	GENERATED_BODY()

	/** Played at the owner's location. Leave empty for no sound. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Feedback")
	TObjectPtr<USoundBase> Sound;

	/** Spawned at the owner's location. Leave empty for no effect. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Feedback")
	TObjectPtr<UNiagaraSystem> Effect;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFeedbackPlayedDelegate, EFeedbackEvent, Event);

/**
 * Add to any actor (player, enemy) to give it sound and VFX feedback.
 * Binds itself to the owner's UHealthComponent and, for characters, to the dash event,
 * so the owner needs no code for it (Observer + Component).
 */
UCLASS(ClassGroup = (TTT), meta = (BlueprintSpawnableComponent))
class TTT_API UFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFeedbackComponent();

	/** Played when the owner takes damage and survives. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Feedback")
	FFeedbackCue HitCue;

	/** Played when the owner's health reaches zero. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Feedback")
	FFeedbackCue DeathCue;

	/** Played when the owner dashes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Feedback")
	FFeedbackCue DashCue;

	/** Fired after a cue played. Bind in Blueprint for extra presentation (camera shake, screen flash). */
	UPROPERTY(BlueprintAssignable, Category = "Feedback|Events")
	FFeedbackPlayedDelegate OnFeedbackPlayed;

	/** Plays the cue for Event at the owner's location and fires OnFeedbackPlayed. */
	UFUNCTION(BlueprintCallable, Category = "Feedback")
	void PlayFeedback(EFeedbackEvent Event);

protected:
	virtual void BeginPlay() override;

private:
	/** Health seen in the last change, to tell damage apart from healing */
	float LastKnownHealth = 0.0f;

	const FFeedbackCue& GetCue(EFeedbackEvent Event) const;

	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void HandleDashed();
};

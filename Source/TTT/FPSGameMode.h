// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "FPSGameMode.generated.h"

/**
 * 
 */
class ACollectiblePickup;
class AShooterCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerUpdatedDelegate, float, RemainingTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCollectibleProgressDelegate, int32, Collected, int32, Total);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundEventDelegate);

/** The three phases of a round. */
UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	PreGame		UMETA(ToolTip = "Waiting for the player to press Start."),
	InGame		UMETA(ToolTip = "Countdown is running."),
	PostGame	UMETA(ToolTip = "Round ended by a win or a loss.")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChangedDelegate, EGamePhase, NewPhase);

UCLASS()

class TTT_API AFPSGameMode : public AGameMode
{
	GENERATED_BODY()
	
	
public:
	AFPSGameMode();
	/** How long the player has to collect every item, in seconds. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timer", meta = (ClampMin = 1, Units = "s"))
	float GameDuration = 60.0f;

	/** Seconds left in the current round (runtime only). */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Timer")
	float RemainingTime;

	/** True once the round has ended, by a win or a loss (runtime only). Same as GetGamePhase() == PostGame; kept for existing Blueprints. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Game State")
	bool bIsRoundOver = false;

	/** Fired every second with the time left. Bind this in the HUD to show the countdown. */
	UPROPERTY(BlueprintAssignable, Category = "Timer|Events")
	FOnTimerUpdatedDelegate OnTimerUpdated;
	
	// Loss condition when the timer hits 0 (C++ only: Blueprint must not end the round itself)
	void HandleGameLoss();

	/** True once the player pressed Start; the match waits until then (runtime only). */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Game State")
	bool bManualStartRequested = false;

	// Called from the Start button in UI: starts the match and the countdown
	UFUNCTION(BlueprintCallable, Category = "Game State")
	void StartGameMatch();

	/** True when the round ended because every collectible was collected (runtime only). */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Game State")
	bool bPlayerWon = false;

	/** True when the round was lost because the player died, false if the timer ran out (runtime only). */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Game State")
	bool bPlayerDied = false;

	// Called from the Restart button on the game-over / win screen: reloads the current level
	UFUNCTION(BlueprintCallable, Category = "Game State")
	void RestartRound();

	// Engine calls this after spawning the player pawn: we subscribe to its OnDeath here
	virtual void SetPlayerDefaults(APawn* PlayerPawn) override;

	UFUNCTION(BlueprintPure, Category = "Collectibles")
	int32 GetTotalCollectibles() const { return TotalCollectibles; }

	UFUNCTION(BlueprintPure, Category = "Collectibles")
	int32 GetCollectedCount() const { return CollectedCount; }

	// --- Events for UI / feedback (Observer pattern: C++ broadcasts, Blueprint listens) ---

	/** Fired on every collect and once at BeginPlay (0 / Total). Bind this in the HUD to show "2 / 5". */
	UPROPERTY(BlueprintAssignable, Category = "Collectibles|Events")
	FOnCollectibleProgressDelegate OnCollectibleProgress;

	/** Fired when the player pressed Start and the countdown begins. */
	UPROPERTY(BlueprintAssignable, Category = "Game State|Events")
	FOnRoundEventDelegate OnMatchStarted;

	/** Fired once when every collectible was collected in time. Show the win screen here. */
	UPROPERTY(BlueprintAssignable, Category = "Game State|Events")
	FOnRoundEventDelegate OnGameWon;

	/** Fired once when the round is lost (timer reached zero or the player died). Show the game-over screen here; read bPlayerDied for the reason. */
	UPROPERTY(BlueprintAssignable, Category = "Game State|Events")
	FOnRoundEventDelegate OnGameLost;

	/** Current phase of the round: Pre-Game, In-Game or Post-Game. */
	UFUNCTION(BlueprintPure, Category = "Game State")
	EGamePhase GetGamePhase() const { return CurrentPhase; }

	/** Fired on every phase change. Switch HUD / menus / input mode on this. In Post-Game, read bPlayerWon / bPlayerDied for the result. */
	UPROPERTY(BlueprintAssignable, Category = "Game State|Events")
	FOnGamePhaseChangedDelegate OnGamePhaseChanged;

	private:
	EGamePhase CurrentPhase = EGamePhase::PreGame;

	// Only place the phase changes: keeps the engine MatchState in sync and notifies listeners
	void SetGamePhase(EGamePhase NewPhase);

	FTimerHandle CountdownTimerHandle;

	int32 TotalCollectibles = 0;
	int32 CollectedCount = 0;

	//Function called every second by the timer handle
	void UpdateTimer();

	// Bound to every ACollectiblePickup::OnCollected in the level
	UFUNCTION()
	void HandleCollectibleCollected(ACollectiblePickup* Pickup);

	// Win condition: all collectibles collected before the timer ran out
	void HandleGameWin();

	// Bound to the player's AShooterCharacter::OnDeath: dying is a loss
	UFUNCTION()
	void HandlePlayerDeath(AShooterCharacter* DeadCharacter);
	
	protected:
	virtual void BeginPlay() override;

	// Match only leaves WaitingToStart after the player pressed Start
	virtual bool ReadyToStartMatch_Implementation() override;
};

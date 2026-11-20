// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "FPSGameMode.generated.h"

/**
 * 
 */
class ACollectiblePickup;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerUpdatedDelegate, float, RemainingTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCollectibleProgressDelegate, int32, Collected, int32, Total);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundEventDelegate);
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

	/** True once the round has ended, by a win or a loss (runtime only). */
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

	// Bound to every ACheckpoint::ItemPickedUp in the level
	UFUNCTION()
	void OnItemPickedUp();

	/** True when the round ended because every collectible was collected (runtime only). */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Game State")
	bool bPlayerWon = false;

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

	/** Fired once when the timer reached zero. Show the game-over screen here. */
	UPROPERTY(BlueprintAssignable, Category = "Game State|Events")
	FOnRoundEventDelegate OnGameLost;

	private:
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
	
	protected:
	virtual void BeginPlay() override;

	// Match only leaves WaitingToStart after the player pressed Start
	virtual bool ReadyToStartMatch_Implementation() override;

	// Debug message while waiting for the Start button
	virtual void HandleMatchIsWaitingToStart() override;
};

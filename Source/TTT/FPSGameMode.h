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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	float GameDuration = 60.0f;
	
	//Current remaining time
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timer")
	float RemainingTime;
	
	//Delegate broadcasted every second
	// True once the countdown has reached zero and the loss condition fired
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timer")
	bool bIsRoundOver = false;

	UPROPERTY(BlueprintAssignable, Category = "Timer")
	FOnTimerUpdatedDelegate OnTimerUpdated;
	
	//Handles the loss condition when the timer hits 0
	UFUNCTION(BlueprintCallable, Category = "Game State")
	void HandleGameLoss();

	// True once the player pressed Start; gates ReadyToStartMatch
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game State")
	bool bManualStartRequested = false;

	// Called from the Start button in UI: starts the match and the countdown
	UFUNCTION(BlueprintCallable, Category = "Game State")
	void StartGameMatch();

	// Bound to every ACheckpoint::ItemPickedUp in the level
	UFUNCTION()
	void OnItemPickedUp();

	// True when the round ended because every collectible was collected
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game State")
	bool bPlayerWon = false;

	UFUNCTION(BlueprintPure, Category = "Collectibles")
	int32 GetTotalCollectibles() const { return TotalCollectibles; }

	UFUNCTION(BlueprintPure, Category = "Collectibles")
	int32 GetCollectedCount() const { return CollectedCount; }

	// --- Events for UI / feedback (Observer pattern: C++ broadcasts, Blueprint listens) ---

	// Fired on every collect and once at BeginPlay (0 / Total)
	UPROPERTY(BlueprintAssignable, Category = "Collectibles|Events")
	FOnCollectibleProgressDelegate OnCollectibleProgress;

	// Fired when the player pressed Start and the countdown begins
	UPROPERTY(BlueprintAssignable, Category = "Game State|Events")
	FOnRoundEventDelegate OnMatchStarted;

	// Fired once when every collectible was collected in time
	UPROPERTY(BlueprintAssignable, Category = "Game State|Events")
	FOnRoundEventDelegate OnGameWon;

	// Fired once when the timer reached zero
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

// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGameMode.h"

#include "GameFramework/GameMode.h"


AFPSGameMode::AFPSGameMode()
{
	// Single source of truth: start value comes from GameDuration (re-applied in BeginPlay for Blueprint overrides)
	RemainingTime = GameDuration;
	
	
}

void AFPSGameMode::BeginPlay()
{
	Super::BeginPlay();
	RemainingTime = GameDuration;
	OnTimerUpdated.Broadcast(RemainingTime);
	// Countdown no longer starts here: it waits for StartGameMatch() (Start button)
}

bool AFPSGameMode::ReadyToStartMatch_Implementation()
{
	// Call the parent's _Implementation directly (Super::ReadyToStartMatch() would re-dispatch the native event)
	return bManualStartRequested && Super::ReadyToStartMatch_Implementation();
}

void AFPSGameMode::StartGameMatch()
{
	// Ignore repeated presses and presses after the round ended
	if (bIsRoundOver || GetWorldTimerManager().TimerExists(CountdownTimerHandle))
	{
		return;
	}

	bManualStartRequested = true;

	if (GetMatchState() == MatchState::WaitingToStart)
	{
		StartMatch();
	}

	GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &AFPSGameMode::UpdateTimer, 1.0f, true);
}

void AFPSGameMode::UpdateTimer()
{
	if (RemainingTime > 0.0f)
	{
		// Clamp so a fractional GameDuration (e.g. 2.5) never broadcasts a negative time
		RemainingTime = FMath::Max(RemainingTime - 1.0f, 0.0f);
		// Broadcast the updated time to any listeners (like your UI)
		OnTimerUpdated.Broadcast(RemainingTime);
		
		if (RemainingTime <= 0.0f)
		{
			//Stop the timer from ticking further
			GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
			
			//Trigger the loss condition
			HandleGameLoss();
		}
	}
}

void AFPSGameMode::HandleGameLoss()
{
	bIsRoundOver = true;
	UE_LOG(LogTemp, Warning, TEXT("Timer hit 0! Game Over - Loss Condition Triggered."));
	
	// Implement your loss condition here:
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->SetPause(true);
		
	}
}

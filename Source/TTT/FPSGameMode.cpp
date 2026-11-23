// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGameMode.h"

#include "TTT.h"
#include "EngineUtils.h"
#include "CollectiblePickup.h"
#include "ShooterCharacter.h"
#include "Kismet/GameplayStatics.h"


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

	// Count the win-condition collectibles and listen to each of them
	TotalCollectibles = 0;
	CollectedCount = 0;
	for (TActorIterator<ACollectiblePickup> It(GetWorld()); It; ++It)
	{
		It->OnCollected.AddUniqueDynamic(this, &AFPSGameMode::HandleCollectibleCollected);
		++TotalCollectibles;
	}
	OnCollectibleProgress.Broadcast(CollectedCount, TotalCollectibles);
}

void AFPSGameMode::HandleCollectibleCollected(ACollectiblePickup* Pickup)
{
	// Collecting after the round ended (win or loss) changes nothing
	if (bIsRoundOver)
	{
		return;
	}

	++CollectedCount;
	OnCollectibleProgress.Broadcast(CollectedCount, TotalCollectibles);

	if (CollectedCount >= TotalCollectibles)
	{
		HandleGameWin();
	}
}

void AFPSGameMode::HandleGameWin()
{
	bIsRoundOver = true;
	bPlayerWon = true;
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);

	OnGameWon.Broadcast();

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->SetPause(true);
	}
}

void AFPSGameMode::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	// Observer: the character only broadcasts that it died, the GameMode decides what that means
	if (AShooterCharacter* Player = Cast<AShooterCharacter>(PlayerPawn))
	{
		Player->OnDeath.AddUniqueDynamic(this, &AFPSGameMode::HandlePlayerDeath);
	}
}

void AFPSGameMode::HandlePlayerDeath(AShooterCharacter* DeadCharacter)
{
	// Dying after the round already ended (e.g. after a win) changes nothing
	if (bIsRoundOver)
	{
		return;
	}

	bPlayerDied = true;
	HandleGameLoss();
}

void AFPSGameMode::RestartRound()
{
	// Reloading the level resets every actor (timer, pickups, enemies, player) to its placed state
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
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
	OnMatchStarted.Broadcast();
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
	// Only one result per round (a win may already have ended it)
	if (bIsRoundOver)
	{
		return;
	}

	bIsRoundOver = true;
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
	UE_LOG(LogTTT, Log, TEXT("Game Over - %s"), bPlayerDied ? TEXT("player died") : TEXT("time ran out"));
	OnGameLost.Broadcast();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->SetPause(true);
		
	}
}

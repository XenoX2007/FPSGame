// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGameMode.h"

#include "GameFramework/GameMode.h"


AFPSGameMode::AFPSGameMode()
{
	RemainingTime = 60.0f;
	
	
}

void AFPSGameMode::BeginPlay()
{
	Super::BeginPlay();
	RemainingTime = GameDuration;
	OnTimerUpdated.Broadcast(RemainingTime);
	
	GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &AFPSGameMode::UpdateTimer, 1.0f, true);
}

void AFPSGameMode::UpdateTimer()
{
	if (RemainingTime > 0.0f)
	{
		RemainingTime -= 1.0f;
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
	UE_LOG(LogTemp, Warning, TEXT("Timer hit 0! Game Over - Loss Condition Triggered."));
	
	// Implement your loss condition here:
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->SetPause(true);
		
	}
}

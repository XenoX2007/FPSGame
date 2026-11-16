// Copyright Epic Games, Inc. All Rights Reserved.

#include "TTTGameMode.h"



ATTTGameMode::ATTTGameMode()
{
	// stubTArray<AActor*> CheckPoints;
}

void ATTTGameMode::BeginPlay()

{
	Super::BeginPlay();
	TArray<AActor*> Checkpoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACheckpoint::StaticClass(), Checkpoints);
	
	for (AActor* Actor : Checkpoints)
	{
		if (ACheckpoint* cp = Cast<ACheckpoint>(Actor))
		{
			cp->ItemPickedUp.AddDynamic(this, &ATTTGameMode::OnItemPickedUp);
		}
	}
	
}
void ATTTGameMode::OnItemPickedUp()
{
	UE_LOG(LogTemp, Warning, TEXT("ItemPickedUp"));
}

void ATTTGameMode::StartGameMatch()
{
	bManualStartRequested = true;
	
	if (GetMatchState() == MatchState::WaitingToStart)
	{
		StartMatch();
	}
}


bool ATTTGameMode::ReadyToStartMatch_Implementation()
{
	return bManualStartRequested && Super::ReadyToStartMatch_Implementation();
}

void ATTTGameMode::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
	FString Message = TEXT("STATE: Pre-Game (Waiting for button press / MatchState::WaitingToStart)");
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, Message);
	}
}

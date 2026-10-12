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
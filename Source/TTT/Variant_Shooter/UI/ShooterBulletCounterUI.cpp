// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterBulletCounterUI.h"
#include "Components/TextBlock.h"
#include "FPSGameMode.h"
#include "Kismet/GameplayStatics.h"

void UShooterBulletCounterUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (AFPSGameMode* GameMode = Cast<AFPSGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GameMode->OnTimerUpdated.AddDynamic(this, &UShooterBulletCounterUI::HandleTimerUpdated);
		
		HandleTimerUpdated(GameMode->RemainingTime);
	}
}

void UShooterBulletCounterUI::HandleTimerUpdated(float RemainingTime)
{
	if (TimerText)
	{
		int Seconds = FMath::CeilToInt(RemainingTime);
		FString TimeString = FString::FromInt(Seconds);
		
		TimerText->SetText(FText::FromString(TimeString));
	}
}

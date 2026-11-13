// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FPSGameMode.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerUpdatedDelegate, float, RemainingTime);
UCLASS()

class TTT_API AFPSGameMode : public AGameModeBase
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
	
	private:
	FTimerHandle CountdownTimerHandle;
	
	//Function called every second by the timer handle
	void UpdateTimer();
	
	protected:
	virtual void BeginPlay() override;
};

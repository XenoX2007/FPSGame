// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Private/Checkpoint.h"
#include "Kismet/GameplayStatics.h"
#include "TTTGameMode.generated.h"

/**
 *  Simple GameMode for a first person game
 */
UCLASS(abstract)
class ATTTGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ATTTGameMode();
	
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnItemPickedUp();
	
	UFUNCTION(BlueprintCallable, Category = "Game State")
	void StartGameMatch();
	
	bool bManualStartRequested = false;
	
	virtual bool ReadyToStartMatch_Implementation() override;
	

	
protected:
	virtual void HandleMatchIsWaitingToStart() override;
};




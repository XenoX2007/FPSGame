// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Private/Checkpoint.h"
#include "Kismet/GameplayStatics.h"
#include "TTTGameMode.generated.h"

/**
 *  Simple GameMode for a first person game
 */
UCLASS(abstract)
class ATTTGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATTTGameMode();
	
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnItemPickedUp();
};




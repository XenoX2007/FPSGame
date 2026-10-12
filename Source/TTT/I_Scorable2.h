// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "I_Scorable2.generated.h"


// This class does not need to be modified.
UINTERFACE()
class UI_Scorable2 : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TTT_API II_Scorable2
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddScore();
};

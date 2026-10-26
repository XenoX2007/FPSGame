// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterBulletCounterUI.generated.h"

/**
 *  Simple bullet counter UI widget for a first person shooter game
 */
UCLASS(abstract)
class TTT_API UShooterBulletCounterUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// Called when the widget is initialized and added to the viewport 
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void HandleTimerUpdated(float RemainingTime);

	/** Allows Blueprint to update sub-widgets with the new bullet count */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "UpdateBulletCounter"))
	void BP_UpdateBulletCounter(int32 MagazineSize, int32 BulletCount);

	/** Allows Blueprint to update sub-widgets with the new life total and play a damage effect on the HUD */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "Damaged"))
	void BP_Damaged(float LifePercent);
	
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "AmourDamaged"))
	void BP_ArmourDamaged(float ArmourPercent);
	
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TimerText;
};



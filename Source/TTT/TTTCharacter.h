// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "TTTCharacter.generated.h"


class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A basic first person character
 */
UCLASS(abstract)
class ATTTCharacter : public ACharacter
{
	GENERATED_BODY()
	

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

protected:

	/** Jump to Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> MouseLookAction;
	
	/** Dash Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")	
	TObjectPtr<UInputAction> DashAction;

	/** Launch speed of the dash, in the direction the player is moving (forward if standing still). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash", meta = (ClampMin = 0, Units = "cm/s"))
	float DashSpeed = 2500.0f;

	/** Seconds before the player can dash again. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash", meta = (ClampMin = 0, Units = "s"))
	float DashCooldown = 1.0f;

private:
	bool bCanDash = true;
	FTimerHandle DashCooldownTimer;

	// Cooldown timer callback
	void ResetDash();

protected:

	
public:
	ATTTCharacter();
	

protected:

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

public:
	/** Launches the character in its move direction; ignored while the dash is on cooldown. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoDash();

	/** False while the dash is on cooldown (use it for a HUD cooldown icon). */
	UFUNCTION(BlueprintPure, Category="Dash")
	bool IsDashReady() const { return bCanDash; }

protected:

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	

public:

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};


// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShooterArmourComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FArmourChangedDelegate, float, CurrentArmour, float, MaxArmour);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TTT_API UShooterArmourComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UShooterArmourComponent();
	
	
	
	/** Fired whenever armour changes (damage or pickup). Bind this in the HUD to update the armour bar. */
	UPROPERTY(BlueprintAssignable, Category = "Armour|Events")
	FArmourChangedDelegate OnArmourChanged;
	
	UFUNCTION(BlueprintCallable, Category = "Armour")
	float MitigateDamage(float incomingDamage);
	
	UFUNCTION(BlueprintCallable, Category = "Armour")
	void AddArmour(float amount);

	UFUNCTION(BlueprintPure, Category = "Armour")
	float GetCurrentArmour() const { return CurrentArmour; }

	UFUNCTION(BlueprintPure, Category = "Armour")
	float GetMaxArmour() const { return MaxArmour; }

	UFUNCTION(BlueprintPure, Category = "Armour")
	bool IsArmourFull() const { return CurrentArmour >= MaxArmour; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	/** Highest armour the player can have. Shield pickups cannot go above this. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armour", meta = (ClampMin = 0))
	float MaxArmour = 100.0f;

	/** Armour at BeginPlay (clamped to MaxArmour). Lower it to make shield pickups matter. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armour", meta = (ClampMin = 0))
	float StartingArmour = 100.0f;

	/** Armour left right now (runtime only). */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Armour")
	float CurrentArmour;

	/** Share of incoming damage the armour soaks up (0.5 = half). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armour", meta = (ClampMin = 0, ClampMax = 1))
	float DamageAbsorption = 0.5f;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};

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
	
	
	
	UPROPERTY(BlueprintReadWrite, Category = "Armour")
	FArmourChangedDelegate OnArmourChanged;
	
	UFUNCTION(BlueprintCallable, Category = "Armour")
	float MitigateDamage(float incomingDamage);
	
	UFUNCTION(BlueprintCallable, Category = "Armour")
	void AddArmour(float amount);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Armour")
	float MaxArmour = 100.0f;
	
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "Armour")
	float CurrentArmour;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Armour")
	float DamageAbsorption = 0.5f;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterArmourComponent.h"

#include "ShooterCharacter.h"		


// Sets default values for this component's properties
UShooterArmourComponent::UShooterArmourComponent()
{
	CurrentArmour = MaxArmour;
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UShooterArmourComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UShooterArmourComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UShooterArmourComponent::AddArmour(float amount)
{
	CurrentArmour = FMath::Clamp(CurrentArmour + amount, 0.0f, MaxArmour);
	OnArmourChanged.Broadcast(CurrentArmour, MaxArmour);
}

float UShooterArmourComponent::MitigateDamage(float incomingDamage)
{
	if (CurrentArmour <= 0.0f)
	{
		return incomingDamage; // if no armour left,all damage gets returned
	}
	
	//Calculate how much damage the armour absorbs
	float DamageToArmour = incomingDamage * DamageAbsorption;
	float RemainingDamage = incomingDamage - DamageToArmour;
	
	if (CurrentArmour >= DamageToArmour)
	{
		//Armour absorbs the damage completely
		CurrentArmour -= DamageToArmour;
		
	}
	else
	{
		//Armour breaks, extra damage goes through
		float ExtraDamageToHealth = (DamageToArmour - CurrentArmour);
		CurrentArmour = 0.0f;
		RemainingDamage += ExtraDamageToHealth;
	}
	
	OnArmourChanged.Broadcast(CurrentArmour, MaxArmour);
	
	return RemainingDamage;
}


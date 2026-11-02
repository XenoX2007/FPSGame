// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterWeaponHolder.h"
#include "TTTCharacter.h"
#include "EnemyCharacter.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSimplePawnDeathDelegate);
class AShooterWeapon;
UCLASS()
class TTT_API AEnemyCharacter : public ATTTCharacter , public IShooterWeaponHolder
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyCharacter();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float MaxHP = 100.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage")
	float CurrentHP ;
	
	//weapon handling
	UPROPERTY(EditAnywhere,Category = "Weapon")
	TSubclassOf<AShooterWeapon> WeaponClass;
	
	UPROPERTY()
	TObjectPtr<AShooterWeapon> Weapon;
	
	UPROPERTY()
	TObjectPtr<AActor> CurrentAimTarget;
	
	void StartFiring(AActor* ActorToShoot);
	void StopFiring();
	
	FSimplePawnDeathDelegate OnPawnDeath;
	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	virtual void AttachWeaponMeshes(AShooterWeapon* Weapon) override;

	/** Plays the firing montage for the weapon */
	virtual void PlayFiringMontage(UAnimMontage* Montage) override {};

	/** Applies weapon recoil to the owner */
	virtual void AddWeaponRecoil(float Recoil) override {};

	/** Updates the weapon's HUD with the current ammo count */
	virtual void UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize) override{};

	/** Calculates and returns the aim location for the weapon */
	virtual FVector GetWeaponTargetLocation() override;

	/** Gives a weapon of this class to the owner */
	virtual void AddWeaponClass(const TSubclassOf<AShooterWeapon>& InWeaponClass) override{};

	/** Activates the passed weapon */
	virtual void OnWeaponActivated(AShooterWeapon* InWeapon) override{};

	/** Deactivates the passed weapon */
	virtual void OnWeaponDeactivated(AShooterWeapon* InWeapon) override{};

	
	virtual void OnSemiWeaponRefire() override{};

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void Die();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};

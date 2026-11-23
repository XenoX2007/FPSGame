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
	
	/** Health the enemy spawns with. The enemy dies when it reaches 0. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = 1))
	float MaxHP = 100.0f;

	/** Health left right now (runtime only). */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Damage")
	float CurrentHP = 0.0f;

	/** Weapon Blueprint the enemy spawns holding (attached to the HandGrip_R socket). */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AShooterWeapon> WeaponClass;

	/** With no target, the enemy aims at a point this far straight ahead. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = 0, Units = "cm"))
	float DefaultAimDistance = 1000.0f;

	UPROPERTY()
	TObjectPtr<AShooterWeapon> Weapon;
	
	UPROPERTY()
	TObjectPtr<AActor> CurrentAimTarget;
	
	void StartFiring(AActor* ActorToShoot);
	void StopFiring();
	
	/** Fired once when HP reaches zero, just before the enemy is destroyed. Bind in the Blueprint for death VFX/SFX. */
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
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

	
	/** Semi-auto weapons fire one shot per StartFiring; re-trigger while we still have a target */
	virtual void OnSemiWeaponRefire() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void Die();
};

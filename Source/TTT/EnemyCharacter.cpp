// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"

#include "ShooterWeapon.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
	
	CurrentHP = MaxHP;
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyCharacter::StartFiring(AActor* ActorToShoot)
{
	CurrentAimTarget = ActorToShoot;
	if (Weapon)
	{
		Weapon->StartFiring();
	}
}

void AEnemyCharacter::StopFiring()
{
	CurrentAimTarget = nullptr;
	if (Weapon)
	{
		Weapon->StopFiring();
	}
}

void AEnemyCharacter::OnSemiWeaponRefire()
{
	// Same idea as AShooterNPC: keep shooting until StopFiring clears the target
	if (Weapon && IsValid(CurrentAimTarget))
	{
		Weapon->StartFiring();
	}
}

float AEnemyCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                                  class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	CurrentHP -= ActualDamage;
	if (CurrentHP <= 0.0f)
	{
		Die();
	}
	return ActualDamage;
}

void AEnemyCharacter::AttachWeaponMeshes(AShooterWeapon* WeaponToAttach)
{
	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, true);
	WeaponToAttach->AttachToActor(this, AttachmentRule);
	WeaponToAttach->GetThirdPersonMesh()->AttachToComponent(GetMesh(),AttachmentRule, FName("HandGrip_R"));
}

FVector AEnemyCharacter::GetWeaponTargetLocation()
{
	// Aim at the current target if we have one, otherwise straight ahead
	if (IsValid(CurrentAimTarget))
	{
		return CurrentAimTarget->GetActorLocation();
	}
	return GetActorLocation() + (GetActorForwardVector() * 1000.0f);
}


// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsValid(WeaponClass))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		Weapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), SpawnParams);
		if (Weapon)
		{
			FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, true);
			Weapon->AttachToActor(this, AttachmentRule);
			
			Weapon->GetThirdPersonMesh()->AttachToComponent(GetMesh(),AttachmentRule, FName("HandGrip_R"));
		}
	}
	
}

void AEnemyCharacter::Die()
{
	StopFiring();
	OnPawnDeath.Broadcast();
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->StopMovementImmediately();
	Destroy();
}

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


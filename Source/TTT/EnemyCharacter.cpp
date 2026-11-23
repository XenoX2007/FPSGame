// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"

#include "ShooterWeapon.h"
#include "HealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	// Movement, AI and shooting are driven by components, the Behaviour Tree and weapon timers: no per-frame C++ work
	PrimaryActorTick.bCanEverTick = false;

	// Same component the player uses (default Max Health 100)
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void AEnemyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	HealthComponent->OnHealthDepleted.AddUniqueDynamic(this, &AEnemyCharacter::HandleHealthDepleted);
}

void AEnemyCharacter::HandleHealthDepleted(AActor* DeadActor)
{
	Die();
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
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// At 0 the component broadcasts OnHealthDepleted -> HandleHealthDepleted -> Die()
	return HealthComponent->ApplyDamage(ActualDamage);
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
	return GetActorLocation() + (GetActorForwardVector() * DefaultAimDistance);
}


// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	// HealthComponent resets its own health in its BeginPlay (inside Super)
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


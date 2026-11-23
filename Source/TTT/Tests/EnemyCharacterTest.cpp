// Automation tests for AEnemyCharacter health (via UHealthComponent) and its Blueprint-facing death event.
// Run: Automation RunTests TTT.Enemy

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "EnemyCharacter.h"
#include "HealthComponent.h"

#if WITH_AUTOMATION_TESTS

namespace TTTEnemyTest
{
	// Spawns an enemy whose health component's MaxHealth is set before BeginPlay,
	// the same way a Blueprint child's default value is applied.
	AEnemyCharacter* SpawnEnemy(FAutomationTestBase& Test, UWorld* World, float MaxHealth)
	{
		AEnemyCharacter* Enemy = World->SpawnActorDeferred<AEnemyCharacter>(AEnemyCharacter::StaticClass(),
			FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!Test.TestNotNull(TEXT("Enemy spawned"), Enemy)) return nullptr;

		UHealthComponent* Health = Enemy->FindComponentByClass<UHealthComponent>();
		if (!Test.TestNotNull(TEXT("Enemy has a HealthComponent"), Health)) return nullptr;
		Health->MaxHealth = MaxHealth;

		Enemy->FinishSpawning(FTransform::Identity); // triggers BeginPlay
		return Enemy;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTEnemyStartsAtMaxHPTest, "TTT.Enemy.StartsAtDesignerMaxHP",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTEnemyStartsAtMaxHPTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	AEnemyCharacter* Enemy = TTTEnemyTest::SpawnEnemy(*this, Wrapper.GetTestWorld(), 250.0f);
	if (!Enemy) return false;
	UHealthComponent* Health = Enemy->FindComponentByClass<UHealthComponent>();

	TestEqual(TEXT("Health uses the MaxHealth set by the designer"), Health->GetCurrentHealth(), 250.0f);

	// 150 damage must not kill an enemy the designer gave 250 HP.
	Enemy->TakeDamage(150.0f, FDamageEvent(), nullptr, nullptr);
	TestEqual(TEXT("Health after 150 damage"), Health->GetCurrentHealth(), 100.0f);
	TestFalse(TEXT("Enemy still alive"), Enemy->IsActorBeingDestroyed());

	Wrapper.EndPlayInTestWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTEnemyDiesTest, "TTT.Enemy.DiesWhenHealthDepleted",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTEnemyDiesTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	AEnemyCharacter* Enemy = TTTEnemyTest::SpawnEnemy(*this, Wrapper.GetTestWorld(), 100.0f);
	if (!Enemy) return false;

	// HealthComponent reports depletion -> the enemy runs Die() -> Destroy()
	Enemy->TakeDamage(1000.0f, FDamageEvent(), nullptr, nullptr);
	TestTrue(TEXT("Enemy destroyed when health hits 0"), Enemy->IsActorBeingDestroyed());

	Wrapper.EndPlayInTestWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTEnemyDeathEventForBlueprintTest, "TTT.Enemy.DeathEventIsBlueprintAssignable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTEnemyDeathEventForBlueprintTest::RunTest(const FString& Parameters)
{
	// Brief: C++ broadcasts, derived Blueprints catch the event for VFX/SFX feedback.
	// Without UPROPERTY(BlueprintAssignable) the delegate is invisible to reflection and to Blueprint.
	const FProperty* Prop = AEnemyCharacter::StaticClass()->FindPropertyByName(TEXT("OnPawnDeath"));
	if (!TestNotNull(TEXT("OnPawnDeath is exposed to reflection (UPROPERTY)"), Prop)) return false;

	TestTrue(TEXT("OnPawnDeath is BlueprintAssignable"), Prop->HasAnyPropertyFlags(CPF_BlueprintAssignable));
	return true;
}

#endif // WITH_AUTOMATION_TESTS

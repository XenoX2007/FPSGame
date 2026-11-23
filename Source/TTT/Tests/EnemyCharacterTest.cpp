// Automation tests for AEnemyCharacter health.
// Run: Automation RunTests TTT.Enemy

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "EnemyCharacter.h"

#if WITH_AUTOMATION_TESTS

namespace TTTEnemyTest
{
	// Spawns an enemy with a custom MaxHP set before BeginPlay,
	// the same way a Blueprint child's default value is applied.
	AEnemyCharacter* SpawnEnemy(UWorld* World, float MaxHP)
	{
		AEnemyCharacter* Enemy = World->SpawnActorDeferred<AEnemyCharacter>(AEnemyCharacter::StaticClass(),
			FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (Enemy)
		{
			Enemy->MaxHP = MaxHP;
			Enemy->FinishSpawning(FTransform::Identity); // triggers BeginPlay
		}
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

	AEnemyCharacter* Enemy = TTTEnemyTest::SpawnEnemy(Wrapper.GetTestWorld(), 250.0f);
	if (!TestNotNull(TEXT("Enemy spawned"), Enemy)) return false;

	TestEqual(TEXT("CurrentHP uses the MaxHP set by the designer"), Enemy->CurrentHP, 250.0f);

	// 150 damage must not kill an enemy the designer gave 250 HP.
	Enemy->TakeDamage(150.0f, FDamageEvent(), nullptr, nullptr);
	TestEqual(TEXT("HP after 150 damage"), Enemy->CurrentHP, 100.0f);
	TestTrue(TEXT("Enemy still alive"), IsValid(Enemy));

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

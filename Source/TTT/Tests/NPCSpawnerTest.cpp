// Automation tests for AShooterNPCSpawner: enemy death (HealthComponent -> Die -> OnPawnDeath) drives respawning.
// Run: Automation RunTests TTT.Spawner

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "ShooterNPCSpawner.h"
#include "EnemyCharacter.h"

#if WITH_AUTOMATION_TESTS

namespace TTTSpawnerTest
{
	// Spawner settings are protected designer values, so set them through reflection like a Blueprint default.
	void SetSpawnerValues(AShooterNPCSpawner* Spawner, int32 SpawnCount)
	{
		UClass* Class = Spawner->GetClass();
		FindFProperty<FClassProperty>(Class, TEXT("NPCClass"))->SetPropertyValue_InContainer(Spawner, AEnemyCharacter::StaticClass());
		FindFProperty<FIntProperty>(Class, TEXT("SpawnCount"))->SetPropertyValue_InContainer(Spawner, SpawnCount);
		FindFProperty<FFloatProperty>(Class, TEXT("InitialSpawnDelay"))->SetPropertyValue_InContainer(Spawner, 1.0f);
		FindFProperty<FFloatProperty>(Class, TEXT("RespawnDelay"))->SetPropertyValue_InContainer(Spawner, 1.0f);
	}

	void TickSeconds(UWorld* World, int32 Seconds)
	{
		for (int32 i = 0; i < Seconds; ++i)
		{
			++GFrameCounter;
			World->GetTimerManager().Tick(1.01f);
		}
	}

	// Living enemies only: a destroyed enemy stays in the iterator until GC
	TArray<AEnemyCharacter*> LivingEnemies(UWorld* World)
	{
		TArray<AEnemyCharacter*> Result;
		for (TActorIterator<AEnemyCharacter> It(World); It; ++It)
		{
			if (!It->IsActorBeingDestroyed())
			{
				Result.Add(*It);
			}
		}
		return Result;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTSpawnerRespawnsTest, "TTT.Spawner.RespawnsAfterEnemyDies",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTSpawnerRespawnsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}
	UWorld* World = Wrapper.GetTestWorld();

	AShooterNPCSpawner* Spawner = World->SpawnActorDeferred<AShooterNPCSpawner>(AShooterNPCSpawner::StaticClass(), FTransform::Identity);
	if (!TestNotNull(TEXT("Spawner spawned"), Spawner)) return false;
	TTTSpawnerTest::SetSpawnerValues(Spawner, /*SpawnCount*/ 2);
	Spawner->FinishSpawning(FTransform::Identity); // BeginPlay schedules the first spawn

	TTTSpawnerTest::TickSeconds(World, 2);
	TArray<AEnemyCharacter*> Enemies = TTTSpawnerTest::LivingEnemies(World);
	if (!TestEqual(TEXT("First enemy spawned"), Enemies.Num(), 1)) return false;

	// Kill it through the real damage path
	Enemies[0]->TakeDamage(100000.0f, FDamageEvent(), nullptr, nullptr);
	TestEqual(TEXT("No living enemy right after the kill"), TTTSpawnerTest::LivingEnemies(World).Num(), 0);

	TTTSpawnerTest::TickSeconds(World, 2);
	TestEqual(TEXT("Spawner respawned an enemy after RespawnDelay"), TTTSpawnerTest::LivingEnemies(World).Num(), 1);

	Wrapper.EndPlayInTestWorld();
	return true;
}

#endif // WITH_AUTOMATION_TESTS

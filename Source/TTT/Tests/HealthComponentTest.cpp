// Automation tests for UHealthComponent on its own (no character), so the rules are tested once for everyone.
// Run: Automation RunTests TTT.HealthComponent

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "HealthComponent.h"

#if WITH_AUTOMATION_TESTS

namespace TTTHealthTest
{
	// A plain actor with a health component added at runtime, like a designer adding it in a Blueprint.
	struct FFixture
	{
		FTestWorldWrapper Wrapper;
		AActor* Owner = nullptr;
		UHealthComponent* Health = nullptr;

		bool Setup(FAutomationTestBase& Test, float MaxHealth)
		{
			if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
			{
				Wrapper.ForwardErrorMessages(&Test);
				return false;
			}
			Owner = Wrapper.GetTestWorld()->SpawnActor<AActor>();
			if (!Test.TestNotNull(TEXT("Owner spawned"), Owner)) return false;

			Health = NewObject<UHealthComponent>(Owner);
			Health->MaxHealth = MaxHealth;
			Health->RegisterComponent(); // owner already began play, so this runs the component's BeginPlay
			return Test.TestTrue(TEXT("Component has begun play"), Health->HasBegunPlay());
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTHealthStartsAtMaxTest, "TTT.HealthComponent.StartsAtDesignerMaxHealth",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTHealthStartsAtMaxTest::RunTest(const FString& Parameters)
{
	TTTHealthTest::FFixture F;
	if (!F.Setup(*this, 250.0f)) return false;

	TestEqual(TEXT("Starts at MaxHealth"), F.Health->GetCurrentHealth(), 250.0f);
	TestEqual(TEXT("Percent is 1"), F.Health->GetHealthPercent(), 1.0f);
	TestTrue(TEXT("Full at start"), F.Health->IsFull());
	TestFalse(TEXT("Alive at start"), F.Health->IsDead());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTHealthDamageTest, "TTT.HealthComponent.DamageClampsAtZeroAndKills",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTHealthDamageTest::RunTest(const FString& Parameters)
{
	TTTHealthTest::FFixture F;
	if (!F.Setup(*this, 100.0f)) return false;

	TestEqual(TEXT("Negative damage is ignored"), F.Health->ApplyDamage(-5.0f), 0.0f);
	TestEqual(TEXT("Health unchanged by negative damage"), F.Health->GetCurrentHealth(), 100.0f);

	TestEqual(TEXT("30 damage taken"), F.Health->ApplyDamage(30.0f), 30.0f);
	TestEqual(TEXT("Health after 30 damage"), F.Health->GetCurrentHealth(), 70.0f);

	// Overkill only takes what is left, and health never goes below zero.
	TestEqual(TEXT("Overkill takes the remaining 70"), F.Health->ApplyDamage(500.0f), 70.0f);
	TestEqual(TEXT("Health clamped at 0"), F.Health->GetCurrentHealth(), 0.0f);
	TestTrue(TEXT("Dead at 0"), F.Health->IsDead());

	TestEqual(TEXT("No damage once dead"), F.Health->ApplyDamage(10.0f), 0.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTHealthHealTest2, "TTT.HealthComponent.HealClampedNotWhenFullOrDead",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTHealthHealTest2::RunTest(const FString& Parameters)
{
	TTTHealthTest::FFixture F;
	if (!F.Setup(*this, 100.0f)) return false;

	TestFalse(TEXT("Heal at full health does nothing"), F.Health->Heal(10.0f));

	F.Health->ApplyDamage(50.0f);
	TestTrue(TEXT("Heal when hurt returns true"), F.Health->Heal(20.0f));
	TestEqual(TEXT("Heal adds the amount"), F.Health->GetCurrentHealth(), 70.0f);

	F.Health->Heal(1000.0f);
	TestEqual(TEXT("Heal clamped to MaxHealth"), F.Health->GetCurrentHealth(), 100.0f);

	F.Health->ApplyDamage(1000.0f);
	TestFalse(TEXT("Cannot heal the dead"), F.Health->Heal(10.0f));
	TestEqual(TEXT("Still dead"), F.Health->GetCurrentHealth(), 0.0f);
	return true;
}

#endif // WITH_AUTOMATION_TESTS

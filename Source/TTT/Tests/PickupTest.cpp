// Automation tests for healing, armour and the Health / Shield pickups.
// Run: Automation RunTests TTT.Health ; TTT.Armour ; TTT.Pickup

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "TimerManager.h"
#include "ShooterCharacter.h"
#include "ShooterArmourComponent.h"
#include "HealthPickup.h"
#include "ShieldPickup.h"

#if WITH_AUTOMATION_TESTS

namespace TTTPickupTest
{
	struct FFixture
	{
		FTestWorldWrapper Wrapper;
		UWorld* World = nullptr;
		AShooterCharacter* Character = nullptr;
		UShooterArmourComponent* Armour = nullptr;

		bool Setup(FAutomationTestBase& Test)
		{
			if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
			{
				Wrapper.ForwardErrorMessages(&Test);
				return false;
			}
			World = Wrapper.GetTestWorld();

			// AShooterCharacter is UCLASS(abstract), so spawn the real player Blueprint instead
			UClass* CharacterClass = LoadClass<AShooterCharacter>(nullptr,
				TEXT("/Game/Variant_Shooter/Blueprints/BP_ShooterCharacter.BP_ShooterCharacter_C"));
			if (!Test.TestNotNull(TEXT("BP_ShooterCharacter class loaded"), CharacterClass)) return false;

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Character = World->SpawnActor<AShooterCharacter>(CharacterClass, FTransform::Identity, Params);
			Armour = Character ? Character->FindComponentByClass<UShooterArmourComponent>() : nullptr;

			return Test.TestNotNull(TEXT("Character"), Character)
				&& Test.TestNotNull(TEXT("Armour component"), Armour);
		}

		void Damage(float Amount) const
		{
			Character->TakeDamage(Amount, FDamageEvent(), nullptr, nullptr);
		}

		template <typename TPickup>
		TPickup* SpawnPickup() const
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			// Far away so it never overlaps the character by itself
			return World->SpawnActor<TPickup>(TPickup::StaticClass(), FTransform(FVector(10000.0f, 0.0f, 0.0f)), Params);
		}

		// Advance the world's timers (see FPSGameModeTimerTest.cpp for why GFrameCounter is bumped).
		void TickTimers(float Seconds) const
		{
			++GFrameCounter;
			World->GetTimerManager().Tick(0.0f); // activate pending timers
			++GFrameCounter;
			World->GetTimerManager().Tick(Seconds);
		}
	};
}

// ---------------------------------------------------------------- Health

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTHealthHealTest, "TTT.Health.HealRestoresHPClampedToMax",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTHealthHealTest::RunTest(const FString& Parameters)
{
	TTTPickupTest::FFixture F;
	if (!F.Setup(*this)) return false;

	const float MaxHP = F.Character->GetMaxHP();
	TestEqual(TEXT("Starts at full HP"), F.Character->GetCurrentHP(), MaxHP);
	TestFalse(TEXT("Heal at full HP does nothing"), F.Character->Heal(10.0f));

	F.Damage(100.0f);
	const float Damaged = F.Character->GetCurrentHP();
	TestTrue(TEXT("Precondition: damage lowered HP"), Damaged < MaxHP);

	TestTrue(TEXT("Heal when hurt returns true"), F.Character->Heal(1.0f));
	TestEqual(TEXT("Heal adds the amount"), F.Character->GetCurrentHP(), Damaged + 1.0f);

	F.Character->Heal(100000.0f);
	TestEqual(TEXT("Heal is clamped to MaxHP"), F.Character->GetCurrentHP(), MaxHP);
	return true;
}

// ---------------------------------------------------------------- Armour

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTArmourGettersTest, "TTT.Armour.StartsFullAndReportsFull",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTArmourGettersTest::RunTest(const FString& Parameters)
{
	TTTPickupTest::FFixture F;
	if (!F.Setup(*this)) return false;

	TestEqual(TEXT("Starts at StartingArmour (default = MaxArmour)"), F.Armour->GetCurrentArmour(), F.Armour->GetMaxArmour());
	TestTrue(TEXT("IsArmourFull at start"), F.Armour->IsArmourFull());

	F.Damage(100.0f);
	TestTrue(TEXT("Damage lowers armour"), F.Armour->GetCurrentArmour() < F.Armour->GetMaxArmour());
	TestFalse(TEXT("Not full after damage"), F.Armour->IsArmourFull());
	return true;
}

// ---------------------------------------------------------------- Health pickup

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTHealthPickupHealsTest, "TTT.Pickup.Health.HealsAndIsConsumed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTHealthPickupHealsTest::RunTest(const FString& Parameters)
{
	TTTPickupTest::FFixture F;
	if (!F.Setup(*this)) return false;
	AHealthPickup* Pickup = F.SpawnPickup<AHealthPickup>();
	if (!TestNotNull(TEXT("Pickup"), Pickup)) return false;

	F.Damage(200.0f);
	const float Before = F.Character->GetCurrentHP();

	TestTrue(TEXT("TryPickup succeeds when hurt"), Pickup->TryPickup(F.Character));
	TestEqual(TEXT("HP increased by HealAmount"), F.Character->GetCurrentHP(), Before + Pickup->GetHealAmount());
	TestFalse(TEXT("Pickup is no longer available"), Pickup->IsAvailable());
	TestTrue(TEXT("Pickup is hidden"), Pickup->IsHidden());
	TestFalse(TEXT("Pickup collision disabled"), Pickup->GetActorEnableCollision());

	TestFalse(TEXT("Cannot pick up again while consumed"), Pickup->TryPickup(F.Character));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTHealthPickupFullTest, "TTT.Pickup.Health.NotConsumedAtFullHP",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTHealthPickupFullTest::RunTest(const FString& Parameters)
{
	TTTPickupTest::FFixture F;
	if (!F.Setup(*this)) return false;
	AHealthPickup* Pickup = F.SpawnPickup<AHealthPickup>();
	if (!TestNotNull(TEXT("Pickup"), Pickup)) return false;

	TestFalse(TEXT("TryPickup fails at full HP"), Pickup->TryPickup(F.Character));
	TestTrue(TEXT("Pickup stays available"), Pickup->IsAvailable());
	TestFalse(TEXT("Pickup stays visible"), Pickup->IsHidden());
	return true;
}

// ---------------------------------------------------------------- Shield pickup

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTShieldPickupTest, "TTT.Pickup.Shield.AddsArmourAndIsConsumed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTShieldPickupTest::RunTest(const FString& Parameters)
{
	TTTPickupTest::FFixture F;
	if (!F.Setup(*this)) return false;
	AShieldPickup* Pickup = F.SpawnPickup<AShieldPickup>();
	if (!TestNotNull(TEXT("Pickup"), Pickup)) return false;

	F.Damage(100.0f); // armour absorbs part of it
	const float Before = F.Armour->GetCurrentArmour();
	TestTrue(TEXT("Precondition: armour not full"), Before < F.Armour->GetMaxArmour());

	TestTrue(TEXT("TryPickup succeeds when armour not full"), Pickup->TryPickup(F.Character));
	TestEqual(TEXT("Armour increased (clamped to max)"), F.Armour->GetCurrentArmour(),
		FMath::Min(Before + Pickup->GetArmourAmount(), F.Armour->GetMaxArmour()));
	TestFalse(TEXT("Pickup is no longer available"), Pickup->IsAvailable());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTShieldPickupFullTest, "TTT.Pickup.Shield.NotConsumedWhenArmourFull",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTShieldPickupFullTest::RunTest(const FString& Parameters)
{
	TTTPickupTest::FFixture F;
	if (!F.Setup(*this)) return false;
	AShieldPickup* Pickup = F.SpawnPickup<AShieldPickup>();
	if (!TestNotNull(TEXT("Pickup"), Pickup)) return false;

	TestFalse(TEXT("TryPickup fails with full armour"), Pickup->TryPickup(F.Character));
	TestTrue(TEXT("Pickup stays available"), Pickup->IsAvailable());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTShieldPickupIgnoresTest, "TTT.Pickup.Shield.IgnoresActorsWithoutArmour",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTShieldPickupIgnoresTest::RunTest(const FString& Parameters)
{
	TTTPickupTest::FFixture F;
	if (!F.Setup(*this)) return false;
	AShieldPickup* Pickup = F.SpawnPickup<AShieldPickup>();
	AActor* Rock = F.World->SpawnActor<AActor>();
	if (!TestNotNull(TEXT("Pickup"), Pickup)) return false;

	TestFalse(TEXT("Null actor"), Pickup->TryPickup(nullptr));
	TestFalse(TEXT("Actor without armour component"), Pickup->TryPickup(Rock));
	TestTrue(TEXT("Pickup stays available"), Pickup->IsAvailable());
	return true;
}

// ---------------------------------------------------------------- Respawn (shared base class)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTPickupRespawnTest, "TTT.Pickup.RespawnsAfterRespawnTime",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTPickupRespawnTest::RunTest(const FString& Parameters)
{
	TTTPickupTest::FFixture F;
	if (!F.Setup(*this)) return false;
	AHealthPickup* Pickup = F.SpawnPickup<AHealthPickup>();
	if (!TestNotNull(TEXT("Pickup"), Pickup)) return false;

	F.Damage(200.0f);
	if (!TestTrue(TEXT("Precondition: picked up"), Pickup->TryPickup(F.Character))) return false;

	// No Blueprint override here: the C++ default OnRespawn must re-enable the pickup by itself.
	F.TickTimers(Pickup->GetRespawnTime() + 0.1f);
	TestTrue(TEXT("Available again after RespawnTime"), Pickup->IsAvailable());
	TestFalse(TEXT("Visible again"), Pickup->IsHidden());
	TestTrue(TEXT("Collision re-enabled"), Pickup->GetActorEnableCollision());
	return true;
}

#endif // WITH_AUTOMATION_TESTS

// Automation tests for the time-attack countdown in AFPSGameMode.
// Run: Automation RunTests TTT.Timer

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "FPSGameMode.h"

#if WITH_AUTOMATION_TESTS

namespace TTTTimerTest
{
	// Spawns AFPSGameMode in a running test world with a custom duration (set before BeginPlay).
	AFPSGameMode* SpawnGameMode(UWorld* World, float Duration)
	{
		AFPSGameMode* GameMode = World->SpawnActorDeferred<AFPSGameMode>(AFPSGameMode::StaticClass(), FTransform::Identity);
		if (GameMode)
		{
			GameMode->GameDuration = Duration;
			GameMode->FinishSpawning(FTransform::Identity); // triggers BeginPlay -> starts the 1s timer

			// New timers start as "pending"; a zero-delta tick activates them without advancing time.
			++GFrameCounter;
			World->GetTimerManager().Tick(0.0f);
		}
		return GameMode;
	}

	// Advances the world's timer manager one second at a time so the looping timer fires once per step.
	// We tick FTimerManager directly: a bare test world does not tick timers reliably.
	void TickSeconds(FTestWorldWrapper& Wrapper, int32 Seconds)
	{
		FTimerManager& TimerManager = Wrapper.GetTestWorld()->GetTimerManager();
		for (int32 i = 0; i < Seconds; ++i)
		{
			// FTimerManager ticks at most once per GFrameCounter; the engine loop normally advances it.
			++GFrameCounter;
			// Slightly over 1s: a timer fires only once time has passed its expiry, not when exactly equal.
			TimerManager.Tick(1.01f);
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTTimerStartsAtDurationTest, "TTT.Timer.StartsAtDuration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTTimerStartsAtDurationTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	AFPSGameMode* GameMode = TTTTimerTest::SpawnGameMode(Wrapper.GetTestWorld(), 10.0f);
	if (!TestNotNull(TEXT("GameMode spawned"), GameMode)) return false;

	TestEqual(TEXT("RemainingTime equals GameDuration after BeginPlay"), GameMode->RemainingTime, 10.0f);
	TestFalse(TEXT("Round is not over at start"), GameMode->bIsRoundOver);

	Wrapper.EndPlayInTestWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTTimerCountsDownTest, "TTT.Timer.CountsDownOneSecondPerTick",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTTimerCountsDownTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	AFPSGameMode* GameMode = TTTTimerTest::SpawnGameMode(Wrapper.GetTestWorld(), 10.0f);
	if (!TestNotNull(TEXT("GameMode spawned"), GameMode)) return false;

	TTTTimerTest::TickSeconds(Wrapper, 3);
	TestEqual(TEXT("RemainingTime after 3 seconds"), GameMode->RemainingTime, 7.0f);
	TestFalse(TEXT("Round is not over yet"), GameMode->bIsRoundOver);

	Wrapper.EndPlayInTestWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTTimerEndsRoundAtZeroTest, "TTT.Timer.EndsRoundAtZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTTimerEndsRoundAtZeroTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	AFPSGameMode* GameMode = TTTTimerTest::SpawnGameMode(Wrapper.GetTestWorld(), 3.0f);
	if (!TestNotNull(TEXT("GameMode spawned"), GameMode)) return false;

	TTTTimerTest::TickSeconds(Wrapper, 3);
	TestEqual(TEXT("RemainingTime is zero"), GameMode->RemainingTime, 0.0f);
	TestTrue(TEXT("Round is over when time hits zero"), GameMode->bIsRoundOver);

	// Extra ticks must not keep counting below zero.
	TTTTimerTest::TickSeconds(Wrapper, 2);
	TestEqual(TEXT("RemainingTime stays at zero"), GameMode->RemainingTime, 0.0f);

	Wrapper.EndPlayInTestWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTTimerNeverNegativeTest, "TTT.Timer.NeverGoesNegativeWithFractionalDuration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTTimerNeverNegativeTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	// 2.5 -> 1.5 -> 0.5 -> (would be -0.5 without clamping)
	AFPSGameMode* GameMode = TTTTimerTest::SpawnGameMode(Wrapper.GetTestWorld(), 2.5f);
	if (!TestNotNull(TEXT("GameMode spawned"), GameMode)) return false;

	TTTTimerTest::TickSeconds(Wrapper, 3);
	TestEqual(TEXT("RemainingTime is clamped to zero"), GameMode->RemainingTime, 0.0f);
	TestTrue(TEXT("Round is over"), GameMode->bIsRoundOver);

	Wrapper.EndPlayInTestWorld();
	return true;
}

#endif // WITH_AUTOMATION_TESTS

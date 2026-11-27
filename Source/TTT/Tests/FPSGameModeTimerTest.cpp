// Automation tests for the time-attack countdown in AFPSGameMode.
// Run: Automation RunTests TTT.Timer

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "FPSGameMode.h"
#include "CollectiblePickup.h"
#include "ShooterCharacter.h"
#include "Engine/DamageEvents.h"

#if WITH_AUTOMATION_TESTS

namespace TTTTimerTest
{
	// Spawns AFPSGameMode in a running test world with a custom duration (set before BeginPlay).
	// bStartMatch simulates the player pressing the Start button (UI calls StartGameMatch).
	AFPSGameMode* SpawnGameMode(UWorld* World, float Duration, bool bStartMatch = true)
	{
		AFPSGameMode* GameMode = World->SpawnActorDeferred<AFPSGameMode>(AFPSGameMode::StaticClass(), FTransform::Identity);
		if (GameMode)
		{
			GameMode->GameDuration = Duration;
			GameMode->FinishSpawning(FTransform::Identity); // triggers BeginPlay -> resets RemainingTime

			if (bStartMatch)
			{
				GameMode->StartGameMatch(); // starts the 1s countdown timer
			}

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

// ---- Game state: the match (and its timer) must wait for the Start button ----

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTGameStateTimerWaitsForStartTest, "TTT.GameState.TimerWaitsForStartGameMatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTGameStateTimerWaitsForStartTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	// Player has NOT pressed Start yet.
	AFPSGameMode* GameMode = TTTTimerTest::SpawnGameMode(Wrapper.GetTestWorld(), 10.0f, /*bStartMatch*/ false);
	if (!TestNotNull(TEXT("GameMode spawned"), GameMode)) return false;

	TTTTimerTest::TickSeconds(Wrapper, 3);
	TestFalse(TEXT("Manual start not requested yet"), GameMode->bManualStartRequested);
	TestEqual(TEXT("RemainingTime does not move before Start"), GameMode->RemainingTime, 10.0f);

	Wrapper.EndPlayInTestWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTGameStateStartBeginsCountdownTest, "TTT.GameState.StartGameMatchBeginsCountdown",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTGameStateStartBeginsCountdownTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	AFPSGameMode* GameMode = TTTTimerTest::SpawnGameMode(Wrapper.GetTestWorld(), 10.0f, /*bStartMatch*/ true);
	if (!TestNotNull(TEXT("GameMode spawned"), GameMode)) return false;

	TTTTimerTest::TickSeconds(Wrapper, 3);
	TestTrue(TEXT("Manual start was requested"), GameMode->bManualStartRequested);
	TestEqual(TEXT("RemainingTime after Start + 3 seconds"), GameMode->RemainingTime, 7.0f);

	Wrapper.EndPlayInTestWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTGameStateStartTwiceTest, "TTT.GameState.StartGameMatchTwiceDoesNotDoubleCount",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTGameStateStartTwiceTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	AFPSGameMode* GameMode = TTTTimerTest::SpawnGameMode(Wrapper.GetTestWorld(), 10.0f, /*bStartMatch*/ true);
	if (!TestNotNull(TEXT("GameMode spawned"), GameMode)) return false;

	// Pressing Start a second time (e.g. double click) must not restart or speed up the countdown.
	TTTTimerTest::TickSeconds(Wrapper, 2);
	GameMode->StartGameMatch();
	TTTTimerTest::TickSeconds(Wrapper, 1);
	TestEqual(TEXT("RemainingTime keeps counting from where it was"), GameMode->RemainingTime, 7.0f);

	Wrapper.EndPlayInTestWorld();
	return true;
}

// ---- Win condition: collect every collectible before the timer runs out ----

namespace TTTTimerTest
{
	TArray<ACollectiblePickup*> SpawnCollectibles(UWorld* World, int32 Count)
	{
		TArray<ACollectiblePickup*> Result;
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		for (int32 i = 0; i < Count; ++i)
		{
			// Spread out and far from the origin so nothing overlaps by accident
			Result.Add(World->SpawnActor<ACollectiblePickup>(ACollectiblePickup::StaticClass(),
				FTransform(FVector(10000.0f + i * 500.0f, 0.0f, 0.0f)), Params));
		}
		return Result;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTGameModeCountsCollectiblesTest, "TTT.GameMode.CountsCollectiblesInLevel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTGameModeCountsCollectiblesTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	UWorld* World = Wrapper.GetTestWorld();
	TTTTimerTest::SpawnCollectibles(World, 3);
	AFPSGameMode* GameMode = TTTTimerTest::SpawnGameMode(World, 10.0f, /*bStartMatch*/ false);
	if (!TestNotNull(TEXT("GameMode spawned"), GameMode)) return false;

	TestEqual(TEXT("TotalCollectibles"), GameMode->GetTotalCollectibles(), 3);
	TestEqual(TEXT("CollectedCount starts at 0"), GameMode->GetCollectedCount(), 0);

	Wrapper.EndPlayInTestWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTGameModeWinsTest, "TTT.GameMode.WinsWhenAllCollectiblesCollected",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTGameModeWinsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	UWorld* World = Wrapper.GetTestWorld();
	TArray<ACollectiblePickup*> Items = TTTTimerTest::SpawnCollectibles(World, 3);
	AFPSGameMode* GameMode = TTTTimerTest::SpawnGameMode(World, 10.0f, /*bStartMatch*/ true);
	if (!TestNotNull(TEXT("GameMode spawned"), GameMode)) return false;

	// Simulate the player collecting items (the pickup broadcasts OnCollected).
	Items[0]->OnCollected.Broadcast(Items[0]);
	Items[1]->OnCollected.Broadcast(Items[1]);
	TestEqual(TEXT("2 of 3 collected"), GameMode->GetCollectedCount(), 2);
	TestFalse(TEXT("Not won yet"), GameMode->bPlayerWon);
	TestFalse(TEXT("Round still running"), GameMode->bIsRoundOver);

	TTTTimerTest::TickSeconds(Wrapper, 2);
	Items[2]->OnCollected.Broadcast(Items[2]);
	TestTrue(TEXT("Player won"), GameMode->bPlayerWon);
	TestTrue(TEXT("Round over"), GameMode->bIsRoundOver);

	// Timer must stop once the round is won.
	TTTTimerTest::TickSeconds(Wrapper, 3);
	TestEqual(TEXT("Timer frozen after win"), GameMode->RemainingTime, 8.0f);

	Wrapper.EndPlayInTestWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTGameModeLosesTest, "TTT.GameMode.LosesWhenTimeRunsOutBeforeAllCollected",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTGameModeLosesTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	UWorld* World = Wrapper.GetTestWorld();
	TArray<ACollectiblePickup*> Items = TTTTimerTest::SpawnCollectibles(World, 2);
	AFPSGameMode* GameMode = TTTTimerTest::SpawnGameMode(World, 3.0f, /*bStartMatch*/ true);
	if (!TestNotNull(TEXT("GameMode spawned"), GameMode)) return false;

	Items[0]->OnCollected.Broadcast(Items[0]);
	TTTTimerTest::TickSeconds(Wrapper, 3);
	TestTrue(TEXT("Round over"), GameMode->bIsRoundOver);
	TestFalse(TEXT("Player did not win"), GameMode->bPlayerWon);

	// Collecting after the round ended must not flip the result.
	Items[1]->OnCollected.Broadcast(Items[1]);
	TestFalse(TEXT("Late collect does not win"), GameMode->bPlayerWon);
	TestEqual(TEXT("Late collect not counted"), GameMode->GetCollectedCount(), 1);

	Wrapper.EndPlayInTestWorld();
	return true;
}

namespace TTTTimerTest
{
	// AShooterCharacter is UCLASS(abstract), so spawn the real player Blueprint instead
	AShooterCharacter* SpawnPlayer(FAutomationTestBase& Test, UWorld* World)
	{
		UClass* CharacterClass = LoadClass<AShooterCharacter>(nullptr,
			TEXT("/Game/Variant_Shooter/Blueprints/BP_ShooterCharacter.BP_ShooterCharacter_C"));
		if (!Test.TestNotNull(TEXT("BP_ShooterCharacter class loaded"), CharacterClass)) return nullptr;

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		return World->SpawnActor<AShooterCharacter>(CharacterClass, FTransform::Identity, Params);
	}

	// Enough damage to get through any armour and kill the player
	void Kill(AShooterCharacter* Character)
	{
		Character->TakeDamage(1000000.0f, FDamageEvent(), nullptr, nullptr);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTGameModePlayerDeathLosesTest, "TTT.GameMode.LosesWhenPlayerDies",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTGameModePlayerDeathLosesTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	UWorld* World = Wrapper.GetTestWorld();
	TTTTimerTest::SpawnCollectibles(World, 2);
	AFPSGameMode* GameMode = TTTTimerTest::SpawnGameMode(World, 10.0f, /*bStartMatch*/ true);
	AShooterCharacter* Player = TTTTimerTest::SpawnPlayer(*this, World);
	if (!TestNotNull(TEXT("GameMode spawned"), GameMode) || !TestNotNull(TEXT("Player spawned"), Player)) return false;

	// Same call the engine makes in RestartPlayer() right after spawning the player pawn
	GameMode->SetPlayerDefaults(Player);

	TTTTimerTest::TickSeconds(Wrapper, 2);
	TTTTimerTest::Kill(Player);
	TestTrue(TEXT("Round over when the player dies"), GameMode->bIsRoundOver);
	TestTrue(TEXT("Loss reason is death"), GameMode->bPlayerDied);
	TestFalse(TEXT("Player did not win"), GameMode->bPlayerWon);

	// Countdown must stop once the player is dead.
	TTTTimerTest::TickSeconds(Wrapper, 3);
	TestEqual(TEXT("Timer frozen after death"), GameMode->RemainingTime, 8.0f);

	Wrapper.EndPlayInTestWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTGameModeDeathAfterWinTest, "TTT.GameMode.DeathAfterWinKeepsWin",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTGameModeDeathAfterWinTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	UWorld* World = Wrapper.GetTestWorld();
	TArray<ACollectiblePickup*> Items = TTTTimerTest::SpawnCollectibles(World, 1);
	AFPSGameMode* GameMode = TTTTimerTest::SpawnGameMode(World, 10.0f, /*bStartMatch*/ true);
	AShooterCharacter* Player = TTTTimerTest::SpawnPlayer(*this, World);
	if (!TestNotNull(TEXT("GameMode spawned"), GameMode) || !TestNotNull(TEXT("Player spawned"), Player)) return false;
	GameMode->SetPlayerDefaults(Player);

	Items[0]->OnCollected.Broadcast(Items[0]);
	TestTrue(TEXT("Player won"), GameMode->bPlayerWon);

	// A stray bullet after the win must not turn it into a loss.
	TTTTimerTest::Kill(Player);
	TestTrue(TEXT("Still won"), GameMode->bPlayerWon);
	TestFalse(TEXT("Death not recorded after the round ended"), GameMode->bPlayerDied);

	Wrapper.EndPlayInTestWorld();
	return true;
}

// ---- Game phases: Pre-Game -> In-Game -> Post-Game, owned by the GameMode ----

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTPhaseStartsInPreGameTest, "TTT.GameMode.Phase.StartsInPreGame",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTPhaseStartsInPreGameTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	AFPSGameMode* GameMode = TTTTimerTest::SpawnGameMode(Wrapper.GetTestWorld(), 10.0f, /*bStartMatch*/ false);
	if (!TestNotNull(TEXT("GameMode spawned"), GameMode)) return false;

	TestEqual(TEXT("Phase before Start"), GameMode->GetGamePhase(), EGamePhase::PreGame);

	Wrapper.EndPlayInTestWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTPhaseStartMovesToInGameTest, "TTT.GameMode.Phase.StartMovesToInGame",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTPhaseStartMovesToInGameTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	AFPSGameMode* GameMode = TTTTimerTest::SpawnGameMode(Wrapper.GetTestWorld(), 10.0f, /*bStartMatch*/ true);
	if (!TestNotNull(TEXT("GameMode spawned"), GameMode)) return false;

	TestEqual(TEXT("Phase after Start"), GameMode->GetGamePhase(), EGamePhase::InGame);

	Wrapper.EndPlayInTestWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTPhaseWinMovesToPostGameTest, "TTT.GameMode.Phase.WinMovesToPostGame",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTPhaseWinMovesToPostGameTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	UWorld* World = Wrapper.GetTestWorld();
	TArray<ACollectiblePickup*> Items = TTTTimerTest::SpawnCollectibles(World, 1);
	AFPSGameMode* GameMode = TTTTimerTest::SpawnGameMode(World, 10.0f, /*bStartMatch*/ true);
	if (!TestNotNull(TEXT("GameMode spawned"), GameMode)) return false;

	Items[0]->OnCollected.Broadcast(Items[0]);
	TestEqual(TEXT("Phase after win"), GameMode->GetGamePhase(), EGamePhase::PostGame);

	Wrapper.EndPlayInTestWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTPhaseTimeoutMovesToPostGameTest, "TTT.GameMode.Phase.TimeoutMovesToPostGame",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTPhaseTimeoutMovesToPostGameTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	AFPSGameMode* GameMode = TTTTimerTest::SpawnGameMode(Wrapper.GetTestWorld(), 2.0f, /*bStartMatch*/ true);
	if (!TestNotNull(TEXT("GameMode spawned"), GameMode)) return false;

	TTTTimerTest::TickSeconds(Wrapper, 2);
	TestEqual(TEXT("Phase after time ran out"), GameMode->GetGamePhase(), EGamePhase::PostGame);

	// Pressing Start in Post-Game must not go back to In-Game.
	GameMode->StartGameMatch();
	TestEqual(TEXT("Start ignored in Post-Game"), GameMode->GetGamePhase(), EGamePhase::PostGame);

	Wrapper.EndPlayInTestWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTPhaseEventIsBlueprintAssignableTest, "TTT.GameMode.Phase.EventIsBlueprintAssignable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTPhaseEventIsBlueprintAssignableTest::RunTest(const FString& Parameters)
{
	// Blueprint UI switches screens on this event, so it must be visible to reflection.
	const FProperty* Prop = AFPSGameMode::StaticClass()->FindPropertyByName(TEXT("OnGamePhaseChanged"));
	if (!TestNotNull(TEXT("OnGamePhaseChanged exists"), Prop)) return false;
	TestTrue(TEXT("OnGamePhaseChanged is BlueprintAssignable"), Prop->HasAnyPropertyFlags(CPF_BlueprintAssignable));
	return true;
}

#endif // WITH_AUTOMATION_TESTS

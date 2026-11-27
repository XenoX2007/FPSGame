// Automation tests for the Enhanced Input movement abilities on ATTTCharacter (Dash, Sprint).
// Run: Automation RunTests TTT.Character

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ShooterCharacter.h"

#if WITH_AUTOMATION_TESTS

namespace TTTCharacterTest
{
	// ATTTCharacter / AShooterCharacter are abstract, so spawn the real player Blueprint
	AShooterCharacter* SpawnPlayer(FAutomationTestBase& Test, UWorld* World)
	{
		UClass* CharacterClass = LoadClass<AShooterCharacter>(nullptr,
			TEXT("/Game/Variant_Shooter/Blueprints/BP_ShooterCharacter.BP_ShooterCharacter_C"));
		if (!Test.TestNotNull(TEXT("BP_ShooterCharacter class loaded"), CharacterClass)) return nullptr;

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AShooterCharacter* Player = World->SpawnActor<AShooterCharacter>(CharacterClass, FTransform::Identity, Params);
		if (Player)
		{
			// Unpossessed pawn with no floor has MOVE_None, and Launch() ignores MOVE_None. In game the player is walking.
			Player->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}
		return Player;
	}

	// Advances only the timer manager (same approach as FPSGameModeTimerTest)
	void TickTimers(UWorld* World, float Seconds)
	{
		++GFrameCounter;
		World->GetTimerManager().Tick(Seconds);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTDashLaunchesTest, "TTT.Character.Dash.LaunchesInsteadOfTeleporting",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTDashLaunchesTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	AShooterCharacter* Player = TTTCharacterTest::SpawnPlayer(*this, Wrapper.GetTestWorld());
	if (!Player) return false;

	const FVector StartLocation = Player->GetActorLocation();
	Player->DoDash();

	// Launch goes through CharacterMovement (collides with walls); teleporting would move the actor instantly.
	TestEqual(TEXT("Actor is not teleported"), Player->GetActorLocation(), StartLocation);
	const FVector Launch = Player->GetCharacterMovement()->PendingLaunchVelocity;
	TestTrue(TEXT("Launch points forward when there is no move input"), Launch.X > 0.0f);
	TestEqual(TEXT("Dash is horizontal"), Launch.Z, 0.0);

	Wrapper.EndPlayInTestWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTDashCooldownTest, "TTT.Character.Dash.CooldownBlocksSpam",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTDashCooldownTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	UWorld* World = Wrapper.GetTestWorld();
	AShooterCharacter* Player = TTTCharacterTest::SpawnPlayer(*this, World);
	if (!Player) return false;

	TestTrue(TEXT("Dash ready at start"), Player->IsDashReady());
	Player->DoDash();
	TestFalse(TEXT("Dash not ready right after dashing"), Player->IsDashReady());

	// A second press during the cooldown must not launch again.
	Player->GetCharacterMovement()->PendingLaunchVelocity = FVector::ZeroVector;
	Player->DoDash();
	TestTrue(TEXT("No launch during cooldown"), Player->GetCharacterMovement()->PendingLaunchVelocity.IsZero());

	// Activate the pending timer, then pass a generous cooldown window.
	TTTCharacterTest::TickTimers(World, 0.0f);
	TTTCharacterTest::TickTimers(World, 10.0f);
	TestTrue(TEXT("Dash ready again after the cooldown"), Player->IsDashReady());

	Wrapper.EndPlayInTestWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTSprintTest, "TTT.Character.Sprint.ChangesWalkSpeedWhileHeld",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTSprintTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	AShooterCharacter* Player = TTTCharacterTest::SpawnPlayer(*this, Wrapper.GetTestWorld());
	if (!Player) return false;

	UCharacterMovementComponent* Movement = Player->GetCharacterMovement();
	const float WalkSpeed = Movement->MaxWalkSpeed;

	Player->DoSprintStart();   // key pressed  (ETriggerEvent::Started)
	TestTrue(TEXT("Faster while sprinting"), Movement->MaxWalkSpeed > WalkSpeed);

	Player->DoSprintEnd();     // key released (ETriggerEvent::Completed)
	TestEqual(TEXT("Back to walk speed after release"), Movement->MaxWalkSpeed, WalkSpeed);

	Wrapper.EndPlayInTestWorld();
	return true;
}

#endif // WITH_AUTOMATION_TESTS

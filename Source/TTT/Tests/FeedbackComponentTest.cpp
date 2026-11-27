// Automation tests for UFeedbackComponent (sound + special effects reacting to owner events).
// Run: Automation RunTests TTT.Feedback

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "FeedbackComponent.h"
#include "FeedbackTestRecorder.h"
#include "HealthComponent.h"
#include "EnemyCharacter.h"
#include "ShooterCharacter.h"

#if WITH_AUTOMATION_TESTS

namespace TTTFeedbackTest
{
	// Adds the component at runtime, like "Add Component" in a Blueprint; BeginPlay runs on register.
	UFeedbackComponent* AddFeedback(AActor* Owner, UFeedbackTestRecorder*& OutRecorder)
	{
		UFeedbackComponent* Feedback = NewObject<UFeedbackComponent>(Owner);
		Feedback->RegisterComponent();

		OutRecorder = NewObject<UFeedbackTestRecorder>();
		Feedback->OnFeedbackPlayed.AddDynamic(OutRecorder, &UFeedbackTestRecorder::Record);
		return Feedback;
	}

	AEnemyCharacter* SpawnEnemy(UWorld* World)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		return World->SpawnActor<AEnemyCharacter>(AEnemyCharacter::StaticClass(), FTransform::Identity, Params);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTFeedbackHitTest, "TTT.Feedback.HitCueWhenDamagedNotWhenHealed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTFeedbackHitTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	AEnemyCharacter* Enemy = TTTFeedbackTest::SpawnEnemy(Wrapper.GetTestWorld());
	if (!TestNotNull(TEXT("Enemy spawned"), Enemy)) return false;
	UHealthComponent* Health = Enemy->FindComponentByClass<UHealthComponent>();

	UFeedbackTestRecorder* Recorder = nullptr;
	TTTFeedbackTest::AddFeedback(Enemy, Recorder);

	Health->ApplyDamage(10.0f);
	TestEqual(TEXT("One cue after taking damage"), Recorder->Events.Num(), 1);
	if (Recorder->Events.Num() > 0)
	{
		TestEqual(TEXT("Damage plays the Hit cue"), Recorder->Events[0], EFeedbackEvent::Hit);
	}

	Health->Heal(5.0f);
	TestEqual(TEXT("Healing plays no hit cue"), Recorder->Events.Num(), 1);

	Wrapper.EndPlayInTestWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTFeedbackDeathTest, "TTT.Feedback.DeathCueWhenKilled",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTFeedbackDeathTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	AEnemyCharacter* Enemy = TTTFeedbackTest::SpawnEnemy(Wrapper.GetTestWorld());
	if (!TestNotNull(TEXT("Enemy spawned"), Enemy)) return false;
	UHealthComponent* Health = Enemy->FindComponentByClass<UHealthComponent>();

	UFeedbackTestRecorder* Recorder = nullptr;
	TTTFeedbackTest::AddFeedback(Enemy, Recorder);

	// Lethal hit: the enemy destroys itself in the same frame, the death cue must still play.
	Health->ApplyDamage(Health->GetMaxHealth());
	TestTrue(TEXT("Death cue played"), Recorder->Events.Contains(EFeedbackEvent::Death));
	TestFalse(TEXT("Killing blow plays Death, not Hit"), Recorder->Events.Contains(EFeedbackEvent::Hit));

	Wrapper.EndPlayInTestWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTFeedbackDashTest, "TTT.Feedback.DashCueWhenOwnerDashes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTFeedbackDashTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	UClass* PlayerClass = LoadClass<AShooterCharacter>(nullptr,
		TEXT("/Game/Variant_Shooter/Blueprints/BP_ShooterCharacter.BP_ShooterCharacter_C"));
	if (!TestNotNull(TEXT("BP_ShooterCharacter class loaded"), PlayerClass)) return false;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AShooterCharacter* Player = Wrapper.GetTestWorld()->SpawnActor<AShooterCharacter>(PlayerClass, FTransform::Identity, Params);
	if (!TestNotNull(TEXT("Player spawned"), Player)) return false;
	Player->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	UFeedbackTestRecorder* Recorder = nullptr;
	TTTFeedbackTest::AddFeedback(Player, Recorder);

	Player->DoDash();
	TestTrue(TEXT("Dash cue played"), Recorder->Events.Contains(EFeedbackEvent::Dash));

	// Pressing again during cooldown does not dash, so no second cue.
	Player->DoDash();
	TestEqual(TEXT("No cue for a blocked dash"), Recorder->Events.Num(), 1);

	Wrapper.EndPlayInTestWorld();
	return true;
}

#endif // WITH_AUTOMATION_TESTS

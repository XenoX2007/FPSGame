// Automation tests for ATestAIController's perception -> Blackboard logic.
// Run: Automation RunTests TTT.AI

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISense_Sight.h"
#include "TestAIController.h"

#if WITH_AUTOMATION_TESTS

namespace TTTAITest
{
	const FName TargetKey(TEXT("TargetActor"));

	// Everything one perception test needs, built in a fresh test world.
	struct FAIFixture
	{
		FTestWorldWrapper Wrapper;
		ATestAIController* AIController = nullptr;
		UBlackboardComponent* Blackboard = nullptr;
		APawn* PlayerPawn = nullptr;   // possessed by a PlayerController
		APawn* OtherPawn = nullptr;    // not player controlled (e.g. another enemy)

		bool Setup(FAutomationTestBase& Test)
		{
			if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
			{
				Wrapper.ForwardErrorMessages(&Test);
				return false;
			}
			UWorld* World = Wrapper.GetTestWorld();

			// Blackboards need the world's AI system; a bare test world may not have one yet.
			if (!World->GetAISystem())
			{
				World->CreateAISystem();
			}
			Test.TestNotNull(TEXT("AI system exists"), World->GetAISystem());

			// AI controller possessing a plain pawn (BehaviorTreeAsset is null, so no tree runs)
			AIController = World->SpawnActor<ATestAIController>();
			APawn* EnemyPawn = World->SpawnActor<APawn>();
			AIController->Possess(EnemyPawn);

			// Runtime blackboard asset mirroring BB_SimpleEnemy: TargetActor (Object, base class Actor)
			UBlackboardData* BBData = NewObject<UBlackboardData>(GetTransientPackage());
			if (UBlackboardKeyType_Object* Key = BBData->UpdatePersistentKey<UBlackboardKeyType_Object>(TargetKey))
			{
				Key->BaseClass = AActor::StaticClass();
			}
			// A loaded .uasset does this in PostLoad; an asset built in code must do it by hand.
			BBData->UpdateParentKeys();
			BBData->UpdateKeyIDs();
			const bool bBlackboardReady = AIController->UseBlackboard(BBData, Blackboard);
			Test.TestTrue(TEXT("UseBlackboard succeeded"), bBlackboardReady);

			APlayerController* PC = World->SpawnActor<APlayerController>();
			PlayerPawn = World->SpawnActor<APawn>();
			PC->Possess(PlayerPawn);

			OtherPawn = World->SpawnActor<APawn>();

			return Test.TestNotNull(TEXT("AIController"), AIController)
				&& Test.TestNotNull(TEXT("Blackboard"), Blackboard)
				&& Test.TestTrue(TEXT("PlayerPawn is player controlled"), PlayerPawn && PlayerPawn->IsPlayerControlled())
				&& Test.TestFalse(TEXT("OtherPawn is not player controlled"), OtherPawn->IsPlayerControlled());
		}

		// Builds a sight stimulus like the perception system would send.
		static FAIStimulus MakeSightStimulus(bool bSensed)
		{
			const UAISense& Sight = *GetDefault<UAISense_Sight>();
			return FAIStimulus(Sight, 1.0f, FVector::ZeroVector, FVector::ZeroVector,
				bSensed ? FAIStimulus::SensingSucceeded : FAIStimulus::SensingFailed);
		}

		UObject* GetTarget() const { return Blackboard->GetValueAsObject(TargetKey); }
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTAISetsTargetWhenPlayerSeenTest, "TTT.AI.Perception.SetsTargetWhenPlayerSeen",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTAISetsTargetWhenPlayerSeenTest::RunTest(const FString& Parameters)
{
	TTTAITest::FAIFixture F;
	if (!F.Setup(*this)) return false;

	F.AIController->OnTargetPerceptionUpdated(F.PlayerPawn, TTTAITest::FAIFixture::MakeSightStimulus(true));

	TestEqual(TEXT("TargetActor is the player"), F.GetTarget(), static_cast<UObject*>(F.PlayerPawn));
	TestEqual(TEXT("AI focuses the player"), F.AIController->GetFocusActor(), static_cast<AActor*>(F.PlayerPawn));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTAIClearsTargetWhenPlayerLostTest, "TTT.AI.Perception.ClearsTargetWhenPlayerLost",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTAIClearsTargetWhenPlayerLostTest::RunTest(const FString& Parameters)
{
	TTTAITest::FAIFixture F;
	if (!F.Setup(*this)) return false;

	F.AIController->OnTargetPerceptionUpdated(F.PlayerPawn, TTTAITest::FAIFixture::MakeSightStimulus(true));
	// Precondition: without it this test would pass even if the handler did nothing at all.
	if (!TestEqual(TEXT("Precondition: target set after seeing player"), F.GetTarget(), static_cast<UObject*>(F.PlayerPawn)))
	{
		return false;
	}

	F.AIController->OnTargetPerceptionUpdated(F.PlayerPawn, TTTAITest::FAIFixture::MakeSightStimulus(false));

	TestNull(TEXT("TargetActor cleared after losing sight"), F.GetTarget());
	TestNull(TEXT("Focus cleared"), F.AIController->GetFocusActor());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTAIIgnoresNonPlayerTest, "TTT.AI.Perception.IgnoresNonPlayerActors",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTAIIgnoresNonPlayerTest::RunTest(const FString& Parameters)
{
	TTTAITest::FAIFixture F;
	if (!F.Setup(*this)) return false;

	// Enemies see each other too (sight detects all affiliations) - they must not target each other.
	F.AIController->OnTargetPerceptionUpdated(F.OtherPawn, TTTAITest::FAIFixture::MakeSightStimulus(true));
	TestNull(TEXT("Seeing a non-player does not set a target"), F.GetTarget());

	// ...and losing sight of a non-player must not clear an existing player target.
	F.AIController->OnTargetPerceptionUpdated(F.PlayerPawn, TTTAITest::FAIFixture::MakeSightStimulus(true));
	F.AIController->OnTargetPerceptionUpdated(F.OtherPawn, TTTAITest::FAIFixture::MakeSightStimulus(false));
	TestEqual(TEXT("Player target kept"), F.GetTarget(), static_cast<UObject*>(F.PlayerPawn));
	return true;
}

#endif // WITH_AUTOMATION_TESTS

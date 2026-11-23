// Automation tests for designer-tunable AI values and for not ticking when there is nothing to do.
// Run: Automation RunTests TTT.Code ; TTT.AI.Sight ; TTT.Enemy

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "TestAIController.h"
#include "EnemyCharacter.h"

#if WITH_AUTOMATION_TESTS

namespace TTTTuningTest
{
	// Sets a float UPROPERTY by name, the same way a Blueprint child's default value is applied.
	bool SetFloat(FAutomationTestBase& Test, UObject* Object, FName Name, float Value)
	{
		FFloatProperty* Prop = FindFProperty<FFloatProperty>(Object->GetClass(), Name);
		if (!Test.TestNotNull(*FString::Printf(TEXT("%s is a float UPROPERTY"), *Name.ToString()), Prop)) return false;
		Prop->SetPropertyValue_InContainer(Object, Value);
		return true;
	}

	// True when this Blueprint class has its own Event Tick (an enabled ReceiveTick graph).
	bool BlueprintImplementsTick(const UClass* Class)
	{
		const UFunction* Tick = Class->FindFunctionByName(TEXT("ReceiveTick"));
		return Tick && !Tick->GetOwnerClass()->HasAnyClassFlags(CLASS_Native);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTNoEmptyNativeTickTest, "TTT.Code.NoEmptyNativeTick",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTNoEmptyNativeTickTest::RunTest(const FString& Parameters)
{
	// Neither class does any per-frame work in C++ (timers and events instead), so they must not tick.
	TestFalse(TEXT("AEnemyCharacter does not tick"), GetDefault<AEnemyCharacter>()->PrimaryActorTick.bCanEverTick);
	TestFalse(TEXT("ATestAIController does not tick"), GetDefault<ATestAIController>()->PrimaryActorTick.bCanEverTick);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTBlueprintTickStillWorksTest, "TTT.Code.BlueprintEventTickStillTicks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTBlueprintTickStillWorksTest::RunTest(const FString& Parameters)
{
	// Turning tick off in C++ must not silently break a Blueprint child that uses Event Tick.
	const TCHAR* BlueprintPaths[] = {
		TEXT("/Game/Studentwork/MyEnemyCharacter.MyEnemyCharacter_C"),
		TEXT("/Game/Studentwork/MyTestAIController.MyTestAIController_C")
	};

	for (const TCHAR* Path : BlueprintPaths)
	{
		UClass* Class = LoadClass<AActor>(nullptr, Path);
		if (!TestNotNull(*FString::Printf(TEXT("%s loaded"), Path), Class)) continue;

		if (TTTTuningTest::BlueprintImplementsTick(Class))
		{
			TestTrue(*FString::Printf(TEXT("%s has Event Tick, so it can tick"), Path),
				Class->GetDefaultObject<AActor>()->PrimaryActorTick.bCanEverTick);
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTSightIsDesignerTunableTest, "TTT.AI.Sight.UsesDesignerValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTSightIsDesignerTunableTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}
	UWorld* World = Wrapper.GetTestWorld();
	if (!World->GetAISystem())
	{
		World->CreateAISystem();
	}

	ATestAIController* AI = World->SpawnActorDeferred<ATestAIController>(ATestAIController::StaticClass(), FTransform::Identity);
	if (!TestNotNull(TEXT("AI controller spawned"), AI)) return false;
	bool bSet = TTTTuningTest::SetFloat(*this, AI, TEXT("SightRadius"), 800.0f);
	bSet &= TTTTuningTest::SetFloat(*this, AI, TEXT("LoseSightRadius"), 900.0f);
	bSet &= TTTTuningTest::SetFloat(*this, AI, TEXT("SightAngleDegrees"), 45.0f);
	AI->FinishSpawning(FTransform::Identity); // triggers BeginPlay
	if (!bSet) return false;

	UAIPerceptionComponent* Perception = AI->FindComponentByClass<UAIPerceptionComponent>();
	const UAISenseConfig_Sight* Sight = Perception
		? Cast<UAISenseConfig_Sight>(Perception->GetSenseConfig(UAISense::GetSenseID<UAISense_Sight>()))
		: nullptr;
	if (!TestNotNull(TEXT("Sight sense configured"), Sight)) return false;

	TestEqual(TEXT("Sight radius from designer"), Sight->SightRadius, 800.0f);
	TestEqual(TEXT("Lose sight radius from designer"), Sight->LoseSightRadius, 900.0f);
	TestEqual(TEXT("Vision angle from designer"), Sight->PeripheralVisionAngleDegrees, 45.0f);

	Wrapper.EndPlayInTestWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTEnemyAimDistanceTest, "TTT.Enemy.AimsAtDesignerDistanceWithoutTarget",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTEnemyAimDistanceTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Wrapper;
	if (!Wrapper.CreateTestWorld(EWorldType::Game) || !Wrapper.BeginPlayInTestWorld())
	{
		Wrapper.ForwardErrorMessages(this);
		return false;
	}

	AEnemyCharacter* Enemy = Wrapper.GetTestWorld()->SpawnActorDeferred<AEnemyCharacter>(AEnemyCharacter::StaticClass(),
		FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!TestNotNull(TEXT("Enemy spawned"), Enemy)) return false;
	const bool bSet = TTTTuningTest::SetFloat(*this, Enemy, TEXT("DefaultAimDistance"), 500.0f);
	Enemy->FinishSpawning(FTransform::Identity);
	if (!bSet) return false;

	// No target: the enemy aims straight ahead, DefaultAimDistance away.
	const FVector Expected = Enemy->GetActorLocation() + Enemy->GetActorForwardVector() * 500.0f;
	TestEqual(TEXT("Aim point uses DefaultAimDistance"), Enemy->GetWeaponTargetLocation(), Expected);

	Wrapper.EndPlayInTestWorld();
	return true;
}

#endif // WITH_AUTOMATION_TESTS

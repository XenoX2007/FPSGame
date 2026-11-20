// Automation tests that check the designer-facing UPROPERTY setup (brief: "Designer Perspective").
// Uses the reflection system to inspect specifiers, so a missing Category/ToolTip fails the build check.
// Run: Automation RunTests TTT.Designer

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"
#include "FPSGameMode.h"
#include "PickupBase.h"
#include "HealthPickup.h"
#include "ShieldPickup.h"
#include "CollectiblePickup.h"
#include "EnemyCharacter.h"
#include "TestAIController.h"
#include "ShooterArmourComponent.h"
#include "ShooterNPCSpawner.h"
#include "BTTask_Shoot.h"
#include "ShooterProjectile.h"

#if WITH_AUTOMATION_TESTS && WITH_METADATA

namespace TTTDesignerTest
{
	// Every class written/changed for this project that designers see in the editor.
	TArray<UClass*> AllDesignerClasses()
	{
		return {
			AFPSGameMode::StaticClass(), APickupBase::StaticClass(), AHealthPickup::StaticClass(),
			AShieldPickup::StaticClass(), ACollectiblePickup::StaticClass(), AEnemyCharacter::StaticClass(),
			ATestAIController::StaticClass(), UShooterArmourComponent::StaticClass(),
			AShooterNPCSpawner::StaticClass(), UBTTask_Shoot::StaticClass(), AShooterProjectile::StaticClass()
		};
	}

	// Classes whose values are game rules shared by every instance -> must be EditDefaultsOnly.
	// (Pickups, spawners and BT nodes are placed/configured per instance, so EditAnywhere is allowed there.)
	TArray<UClass*> ClassWideRuleClasses()
	{
		return {
			AFPSGameMode::StaticClass(), AEnemyCharacter::StaticClass(),
			ATestAIController::StaticClass(), UShooterArmourComponent::StaticClass()
		};
	}

	bool IsDesignerFacing(const FProperty* Prop)
	{
		return Prop->HasAnyPropertyFlags(CPF_Edit | CPF_BlueprintVisible | CPF_BlueprintAssignable);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTDesignerCategoryTooltipTest, "TTT.Designer.PropertiesHaveCategoryAndTooltip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTDesignerCategoryTooltipTest::RunTest(const FString& Parameters)
{
	for (UClass* Class : TTTDesignerTest::AllDesignerClasses())
	{
		// ExcludeSuper: only check properties declared in our own class, not engine parents
		for (TFieldIterator<FProperty> It(Class, EFieldIteratorFlags::ExcludeSuper); It; ++It)
		{
			const FProperty* Prop = *It;
			if (!TTTDesignerTest::IsDesignerFacing(Prop)) continue;

			const FString Name = FString::Printf(TEXT("%s::%s"), *Class->GetName(), *Prop->GetName());
			TestTrue(Name + TEXT(" has a Category"), !Prop->GetMetaData(TEXT("Category")).TrimStartAndEnd().IsEmpty());
			TestTrue(Name + TEXT(" has a ToolTip"), !Prop->GetMetaData(TEXT("ToolTip")).IsEmpty());
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTDesignerReadOnlyTest, "TTT.Designer.BlueprintCannotWriteGameData",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTDesignerReadOnlyTest::RunTest(const FString& Parameters)
{
	// Brief: Blueprints may read data and react to events, but not change core logic/data.
	for (UClass* Class : TTTDesignerTest::AllDesignerClasses())
	{
		for (TFieldIterator<FProperty> It(Class, EFieldIteratorFlags::ExcludeSuper); It; ++It)
		{
			const FProperty* Prop = *It;
			if (!Prop->HasAnyPropertyFlags(CPF_BlueprintVisible)) continue;

			const FString Name = FString::Printf(TEXT("%s::%s"), *Class->GetName(), *Prop->GetName());
			TestTrue(Name + TEXT(" is BlueprintReadOnly"), Prop->HasAnyPropertyFlags(CPF_BlueprintReadOnly));
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTDesignerDefaultsOnlyTest, "TTT.Designer.GameRulesAreEditDefaultsOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTDesignerDefaultsOnlyTest::RunTest(const FString& Parameters)
{
	for (UClass* Class : TTTDesignerTest::ClassWideRuleClasses())
	{
		for (TFieldIterator<FProperty> It(Class, EFieldIteratorFlags::ExcludeSuper); It; ++It)
		{
			const FProperty* Prop = *It;
			// Editable (not Visible*) values must only be tuned on the Blueprint class defaults
			if (!Prop->HasAnyPropertyFlags(CPF_Edit) || Prop->HasAnyPropertyFlags(CPF_EditConst)) continue;

			const FString Name = FString::Printf(TEXT("%s::%s"), *Class->GetName(), *Prop->GetName());
			TestTrue(Name + TEXT(" is EditDefaultsOnly"), Prop->HasAnyPropertyFlags(CPF_DisableEditOnInstance));
		}
	}
	return true;
}

#endif // WITH_AUTOMATION_TESTS && WITH_METADATA

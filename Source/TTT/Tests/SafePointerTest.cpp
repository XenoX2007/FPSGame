// Automation test for safe pointer usage (brief, Phase 3: "safe pointer management").
// Uses reflection to find every UPROPERTY object pointer declared in the TTT module and
// checks it is a TObjectPtr<> (or a wrapper like TSubclassOf / TSoftObjectPtr / TWeakObjectPtr), never a raw T*.
// Run: Automation RunTests TTT.Code

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "UObject/UObjectIterator.h"
#include "UObject/UnrealType.h"

#if WITH_AUTOMATION_TESTS

namespace TTTSafePointerTest
{
	// A raw "UFoo*" property has neither flag; TObjectPtr sets CPF_TObjectPtr, other wrappers set CPF_UObjectWrapper.
	bool IsRawObjectPointer(const FProperty* Prop)
	{
		return CastField<FObjectPropertyBase>(Prop) != nullptr
			&& !Prop->HasAnyPropertyFlags(CPF_TObjectPtr | CPF_UObjectWrapper);
	}

	// Checks the property itself plus the element types of TArray / TSet / TMap.
	void CollectRawPointers(const FProperty* Prop, const FString& Owner, TArray<FString>& OutErrors)
	{
		TArray<const FProperty*> ToCheck = { Prop };
		if (const FArrayProperty* Array = CastField<FArrayProperty>(Prop)) ToCheck.Add(Array->Inner);
		if (const FSetProperty* Set = CastField<FSetProperty>(Prop)) ToCheck.Add(Set->ElementProp);
		if (const FMapProperty* Map = CastField<FMapProperty>(Prop)) { ToCheck.Add(Map->KeyProp); ToCheck.Add(Map->ValueProp); }

		for (const FProperty* Checked : ToCheck)
		{
			if (IsRawObjectPointer(Checked))
			{
				OutErrors.Add(FString::Printf(TEXT("%s::%s uses a raw pointer, use TObjectPtr<>"), *Owner, *Prop->GetName()));
				return;
			}
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTTTNoRawUPropertyPointersTest, "TTT.Code.UPropertiesUseTObjectPtr",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTTTNoRawUPropertyPointersTest::RunTest(const FString& Parameters)
{
	const FString ModulePackage = TEXT("/Script/TTT");
	TArray<FString> Errors;
	int32 CheckedTypes = 0;

	// Classes and structs only: UFUNCTION parameters may stay raw pointers.
	for (TObjectIterator<UStruct> It; It; ++It)
	{
		UStruct* Type = *It;
		if (Type->IsA<UFunction>() || Type->GetOutermost()->GetName() != ModulePackage) continue;
		++CheckedTypes;

		// ExcludeSuper: engine parent classes are not ours to change
		for (TFieldIterator<FProperty> PropIt(Type, EFieldIteratorFlags::ExcludeSuper); PropIt; ++PropIt)
		{
			TTTSafePointerTest::CollectRawPointers(*PropIt, Type->GetName(), Errors);
		}
	}

	TestTrue(TEXT("Found TTT classes to check"), CheckedTypes > 0);
	for (const FString& Error : Errors)
	{
		AddError(Error);
	}
	return true;
}

#endif // WITH_AUTOMATION_TESTS

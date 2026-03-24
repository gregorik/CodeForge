// CodeForgeViabilityTest.cpp
// Tests for viability & marketability features.

#include "Misc/AutomationTest.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgeNode_Property.h"
#include "CodeForgeNode_Function.h"
#include "CodeForgeEdGraph.h"
#include "CodeForgeEdGraphSchema.h"
#include "CodeForgeGenerator.h"
#include "CodeForgeTypes.h"
#include "Misc/Paths.h"

#if WITH_DEV_AUTOMATION_TESTS

// ===========================================================================
// SyncFromBlueprint — Property
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCodeForgeTest_SyncFromBlueprint_Property,
	"CodeForge.Viability.SyncFromBlueprint.Property",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_SyncFromBlueprint_Property::RunTest(const FString& Parameters)
{
	UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
	FCodeForgePropertyDef Prop;
	Prop.Name = TEXT("Health");
	Prop.Type = TEXT("float");
	Prop.bEditAnywhere = true;
	BP->Properties.Add(Prop);

	UCodeForgeNode_Property* Node = NewObject<UCodeForgeNode_Property>();
	Node->PropertyDef.Name = TEXT("Health"); // Name must match for lookup
	Node->SyncFromBlueprint(BP);

	TestEqual(TEXT("PropertyDef.Name"), Node->PropertyDef.Name, TEXT("Health"));
	TestEqual(TEXT("PropertyDef.Type"), Node->PropertyDef.Type, TEXT("float"));
	TestTrue(TEXT("PropertyDef.bEditAnywhere"), Node->PropertyDef.bEditAnywhere);
	return true;
}

// ===========================================================================
// SyncFromBlueprint — Function
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCodeForgeTest_SyncFromBlueprint_Function,
	"CodeForge.Viability.SyncFromBlueprint.Function",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_SyncFromBlueprint_Function::RunTest(const FString& Parameters)
{
	UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
	FCodeForgeFunctionDef Func;
	Func.Name = TEXT("FireWeapon");
	Func.ReturnType = TEXT("void");
	BP->Functions.Add(Func);

	UCodeForgeNode_Function* Node = NewObject<UCodeForgeNode_Function>();
	Node->FunctionDef.Name = TEXT("FireWeapon"); // Name must match for lookup
	Node->SyncFromBlueprint(BP);

	TestEqual(TEXT("FunctionDef.Name"), Node->FunctionDef.Name, TEXT("FireWeapon"));
	TestEqual(TEXT("FunctionDef.ReturnType"), Node->FunctionDef.ReturnType, TEXT("void"));
	return true;
}

// ===========================================================================
// Include Paths in Generated Code
// ===========================================================================
static FString GetTemplatePath_Viability()
{
	return FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("CodeForge/Content/Templates"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCodeForgeTest_IncludePathInContext,
	"CodeForge.Viability.IncludePaths",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_IncludePathInContext::RunTest(const FString& Parameters)
{
	UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
	BP->ClassName = TEXT("AMyActor");
	BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
	BP->ClassType = ECodeForgeClassType::Actor;

	FCodeForgePropertyDef Prop;
	Prop.Name = TEXT("Inventory");
	Prop.Type = TEXT("FInventoryData");
	Prop.IncludePath = TEXT("InventoryData.h");
	BP->Properties.Add(Prop);

	FCodeForgePropertyDef Prop2;
	Prop2.Name = TEXT("Health");
	Prop2.Type = TEXT("float");
	// No IncludePath — built-in type
	BP->Properties.Add(Prop2);

	FCodeForgeGenerator Generator;
	Generator.SetTemplatePath(GetTemplatePath_Viability());
	FCodeForgeGeneratorResult Result = Generator.Generate(BP);

	TestTrue(TEXT("Generation succeeded"), Result.bSuccess);
	TestTrue(TEXT("Header contains InventoryData.h include"),
		Result.HeaderContent.Contains(TEXT("#include \"InventoryData.h\"")));
	TestFalse(TEXT("Header does NOT contain empty include"),
		Result.HeaderContent.Contains(TEXT("#include \"\"")));
	return true;
}

// ===========================================================================
// Const Function Generation
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCodeForgeTest_ConstFunctionGeneration,
	"CodeForge.Viability.ConstFunctionGeneration",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_ConstFunctionGeneration::RunTest(const FString& Parameters)
{
	UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
	BP->ClassName = TEXT("AMyActor");
	BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
	BP->ClassType = ECodeForgeClassType::Actor;

	FCodeForgeFunctionDef ConstFunc;
	ConstFunc.Name = TEXT("GetHealth");
	ConstFunc.ReturnType = TEXT("float");
	ConstFunc.bBlueprintPure = true;
	ConstFunc.bConst = true;
	ConstFunc.FunctionBody = TEXT("\treturn Health;");
	BP->Functions.Add(ConstFunc);

	FCodeForgeFunctionDef NonConstFunc;
	NonConstFunc.Name = TEXT("SetHealth");
	NonConstFunc.ReturnType = TEXT("void");
	NonConstFunc.bBlueprintCallable = true;
	NonConstFunc.bConst = false;
	BP->Functions.Add(NonConstFunc);

	// Add a property so class isn't empty
	FCodeForgePropertyDef Prop;
	Prop.Name = TEXT("Health");
	Prop.Type = TEXT("float");
	BP->Properties.Add(Prop);

	FCodeForgeGenerator Generator;
	Generator.SetTemplatePath(GetTemplatePath_Viability());
	FCodeForgeGeneratorResult Result = Generator.Generate(BP);

	TestTrue(TEXT("Success"), Result.bSuccess);
	TestTrue(TEXT("Header contains GetHealth() const"),
		Result.HeaderContent.Contains(TEXT("GetHealth()") + FString(TEXT(" const;"))));
	TestTrue(TEXT("Source contains GetHealth() const"),
		Result.SourceContent.Contains(TEXT("GetHealth()") + FString(TEXT(" const"))));
	TestFalse(TEXT("SetHealth is NOT const in header"),
		Result.HeaderContent.Contains(TEXT("SetHealth()") + FString(TEXT(" const"))));
	return true;
}

// ===========================================================================
// Validation — Class Name Prefix
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCodeForgeTest_Validation_ClassNamePrefix,
	"CodeForge.Viability.Validation.ClassNamePrefix",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_Validation_ClassNamePrefix::RunTest(const FString& Parameters)
{
	// Actor-derived class should start with 'A'
	{
		UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
		BP->ClassName = TEXT("UMyBadActor");
		BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
		BP->ClassType = ECodeForgeClassType::Actor;
		TArray<FCodeForgeValidationResult> Results = BP->Validate();
		bool bHasWarning = false;
		for (const FCodeForgeValidationResult& VR : Results)
		{
			if (VR.Message.Contains(TEXT("should start with 'A'")))
			{
				bHasWarning = true;
				break;
			}
		}
		TestTrue(TEXT("Actor class with 'U' prefix triggers warning"), bHasWarning);
	}

	// Struct should start with 'F'
	{
		UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
		BP->ClassName = TEXT("AMyBadStruct");
		BP->BlueprintKind = ECodeForgeBlueprintKind::Struct;
		TArray<FCodeForgeValidationResult> Results = BP->Validate();
		bool bHasWarning = false;
		for (const FCodeForgeValidationResult& VR : Results)
		{
			if (VR.Message.Contains(TEXT("should start with 'F'")))
			{
				bHasWarning = true;
				break;
			}
		}
		TestTrue(TEXT("Struct with 'A' prefix triggers warning"), bHasWarning);
	}

	// Correct prefix should NOT trigger warning
	{
		UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
		BP->ClassName = TEXT("AMyActor");
		BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
		BP->ClassType = ECodeForgeClassType::Actor;
		TArray<FCodeForgeValidationResult> Results = BP->Validate();
		bool bHasWarning = false;
		for (const FCodeForgeValidationResult& VR : Results)
		{
			if (VR.Message.Contains(TEXT("should start with")))
			{
				bHasWarning = true;
				break;
			}
		}
		TestFalse(TEXT("Correct Actor prefix does NOT trigger warning"), bHasWarning);
	}

	return true;
}

// ===========================================================================
// Validation — Enum Entries
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCodeForgeTest_Validation_EnumEntries,
	"CodeForge.Viability.Validation.EnumEntries",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_Validation_EnumEntries::RunTest(const FString& Parameters)
{
	// Empty enum triggers warning
	{
		UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
		BP->ClassName = TEXT("EMyEnum");
		BP->BlueprintKind = ECodeForgeBlueprintKind::Enum;
		TArray<FCodeForgeValidationResult> Results = BP->Validate();
		bool bHasWarning = false;
		for (const FCodeForgeValidationResult& VR : Results)
		{
			if (VR.Message.Contains(TEXT("no entries")))
			{
				bHasWarning = true;
				break;
			}
		}
		TestTrue(TEXT("Empty enum triggers warning"), bHasWarning);
	}

	// Duplicate entry triggers error
	{
		UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
		BP->ClassName = TEXT("EMyEnum");
		BP->BlueprintKind = ECodeForgeBlueprintKind::Enum;
		FCodeForgeEnumEntryDef Entry1;
		Entry1.Name = TEXT("Value1");
		FCodeForgeEnumEntryDef Entry2;
		Entry2.Name = TEXT("Value1");
		BP->EnumEntries.Add(Entry1);
		BP->EnumEntries.Add(Entry2);
		TArray<FCodeForgeValidationResult> Results = BP->Validate();
		bool bHasError = false;
		for (const FCodeForgeValidationResult& VR : Results)
		{
			if (VR.Message.Contains(TEXT("Duplicate enum entry")))
			{
				bHasError = true;
				break;
			}
		}
		TestTrue(TEXT("Duplicate enum entry triggers error"), bHasError);
	}

	return true;
}

// ===========================================================================
// Function IncludePath Collection
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCodeForgeTest_FunctionIncludePath,
	"CodeForge.Viability.FunctionIncludePath",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_FunctionIncludePath::RunTest(const FString& Parameters)
{
	UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
	BP->ClassName = TEXT("AMyActor");
	BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
	BP->ClassType = ECodeForgeClassType::Actor;

	FCodeForgeFunctionDef Func;
	Func.Name = TEXT("GetInventory");
	Func.ReturnType = TEXT("FInventoryData");
	Func.bBlueprintCallable = true;
	Func.IncludePath = TEXT("InventoryData.h");
	BP->Functions.Add(Func);

	FCodeForgeGenerator Generator;
	Generator.SetTemplatePath(GetTemplatePath_Viability());
	FCodeForgeGeneratorResult Result = Generator.Generate(BP);

	TestTrue(TEXT("Success"), Result.bSuccess);
	TestTrue(TEXT("Header contains InventoryData.h include from function"),
		Result.HeaderContent.Contains(TEXT("#include \"InventoryData.h\"")));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS


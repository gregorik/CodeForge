// CodeForgeEndToEndTest.cpp
// End-to-end automation tests for the CodeForge pipeline — Task 11.
//
// Each test creates a UCodeForgeBlueprint programmatically, validates it,
// runs FCodeForgeGenerator, and verifies the resulting .h/.cpp text.

#include "Misc/AutomationTest.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgeGenerator.h"
#include "CodeForgePropertyDef.h"
#include "CodeForgeFunctionDef.h"
#include "CodeForgeEnumDef.h"
#include "CodeForgeTypes.h"
#include "Misc/Paths.h"

#if WITH_DEV_AUTOMATION_TESTS

// Helper: return the absolute path to the plugin template directory.
static FString GetTemplatePath()
{
	return FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("CodeForge/Content/Templates"));
}

// ===========================================================================
// Test 1 — Full replicated Actor (most comprehensive)
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCodeForgeEndToEndTest_ReplicatedActorFull,
	"CodeForge.EndToEnd.ReplicatedActorFull",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeEndToEndTest_ReplicatedActorFull::RunTest(const FString& Parameters)
{
	// --- Build Blueprint ------------------------------------------------
	UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>(GetTransientPackage());
	BP->ClassName       = TEXT("ATestReplicatedActor");
	BP->BlueprintKind   = ECodeForgeBlueprintKind::Class;
	BP->ClassType       = ECodeForgeClassType::Actor;
	BP->bReplicated     = true;
	BP->ModuleTarget    = TEXT("CodeForge");

	// Property 1 — Health (replicated with notify)
	{
		FCodeForgePropertyDef Prop;
		Prop.Name                 = TEXT("Health");
		Prop.Type                 = TEXT("float");
		Prop.bEditAnywhere        = true;
		Prop.bBlueprintReadWrite  = true;
		Prop.Category             = TEXT("Stats");
		Prop.bReplicated          = true;
		Prop.ReplicationCondition = ECodeForgeRepCondition::OwnerOnly;
		Prop.bRepNotify           = true;
		BP->Properties.Add(Prop);
	}

	// Property 2 — MaxHealth (non-replicated, defaults-only)
	{
		FCodeForgePropertyDef Prop;
		Prop.Name               = TEXT("MaxHealth");
		Prop.Type               = TEXT("float");
		Prop.bEditDefaultsOnly  = true;
		Prop.bBlueprintReadOnly = true;
		Prop.Category           = TEXT("Stats");
		BP->Properties.Add(Prop);
	}

	// Property 3 — DisplayName (with Meta)
	{
		FCodeForgePropertyDef Prop;
		Prop.Name                = TEXT("DisplayName");
		Prop.Type                = TEXT("FString");
		Prop.bEditAnywhere       = true;
		Prop.bBlueprintReadWrite = true;
		Prop.Category            = TEXT("Info");
		Prop.Meta.Add(TEXT("AllowPrivateAccess"), TEXT("true"));
		BP->Properties.Add(Prop);
	}

	// Function 1 — TakeDamage
	{
		FCodeForgeFunctionDef Func;
		Func.Name              = TEXT("TakeDamage");
		Func.ReturnType        = TEXT("void");
		Func.bBlueprintCallable = true;
		Func.Category          = TEXT("Combat");

		FCodeForgeParamDef Param;
		Param.Name   = TEXT("Amount");
		Param.Type   = TEXT("float");
		Param.bIsConst = true;
		Param.bIsRef   = true;
		Func.Parameters.Add(Param);

		BP->Functions.Add(Func);
	}

	// Function 2 — GetHealthPercent (pure, const)
	{
		FCodeForgeFunctionDef Func;
		Func.Name          = TEXT("GetHealthPercent");
		Func.ReturnType    = TEXT("float");
		Func.bBlueprintPure = true;
		Func.bConst         = true;
		Func.Category      = TEXT("Stats");
		BP->Functions.Add(Func);
	}

	// --- Step 1: Validate -----------------------------------------------
	TArray<FCodeForgeValidationResult> Errors = BP->Validate();

	int32 ErrorCount = 0;
	for (const FCodeForgeValidationResult& R : Errors)
	{
		if (R.Severity == ECodeForgeValidationSeverity::Error)
		{
			++ErrorCount;
		}
	}
	TestEqual(TEXT("Validation should report zero errors"), ErrorCount, 0);

	// --- Step 2: Generate -----------------------------------------------
	FCodeForgeGenerator Generator;
	Generator.SetTemplatePath(GetTemplatePath());

	FCodeForgeGeneratorResult Result = Generator.Generate(BP);

	// --- Step 3: Generation should succeed ------------------------------
	TestTrue(TEXT("Generation must succeed"), Result.bSuccess);

	if (!Result.bSuccess)
	{
		AddError(FString::Printf(TEXT("Generation failed: %s"), *Result.ErrorMessage));
		return true;
	}

	// --- Step 4: Header content checks ----------------------------------
	const FString& H = Result.HeaderContent;
	TestTrue(TEXT("Header contains UCLASS("),            H.Contains(TEXT("UCLASS(")));
	TestTrue(TEXT("Header contains GENERATED_BODY()"),   H.Contains(TEXT("GENERATED_BODY()")));
	TestTrue(TEXT("Header contains UPROPERTY("),         H.Contains(TEXT("UPROPERTY(")));
	TestTrue(TEXT("Header contains UFUNCTION("),         H.Contains(TEXT("UFUNCTION(")));
	TestTrue(TEXT("Header contains GetLifetimeReplicatedProps"), H.Contains(TEXT("GetLifetimeReplicatedProps")));
	TestTrue(TEXT("Header contains OnRep_Health"),       H.Contains(TEXT("OnRep_Health")));
	TestTrue(TEXT("Header contains CODEFORGE_API"),      H.Contains(TEXT("CODEFORGE_API")));

	// --- Step 5: Source content checks ----------------------------------
	const FString& S = Result.SourceContent;
	TestTrue(TEXT("Source contains #include"),            S.Contains(TEXT("#include")));
	TestTrue(TEXT("Source contains DOREPLIFETIME"),       S.Contains(TEXT("DOREPLIFETIME")));
	TestTrue(TEXT("Source contains OnRep_Health"),        S.Contains(TEXT("OnRep_Health")));
	TestTrue(TEXT("Source contains constructor"),
		S.Contains(TEXT("ATestReplicatedActor::ATestReplicatedActor")));

	return true;
}

// ===========================================================================
// Test 2 — Struct generation
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCodeForgeEndToEndTest_StructGeneration,
	"CodeForge.EndToEnd.StructGeneration",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeEndToEndTest_StructGeneration::RunTest(const FString& Parameters)
{
	// --- Build Blueprint ------------------------------------------------
	UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>(GetTransientPackage());
	BP->ClassName      = TEXT("FTestStruct");
	BP->BlueprintKind  = ECodeForgeBlueprintKind::Struct;
	BP->bBlueprintType = true;

	// StructProperty 1 — Value
	{
		FCodeForgePropertyDef Prop;
		Prop.Name                = TEXT("Value");
		Prop.Type                = TEXT("int32");
		Prop.bEditAnywhere       = true;
		Prop.bBlueprintReadWrite = true;
		BP->StructProperties.Add(Prop);
	}

	// StructProperty 2 — Label
	{
		FCodeForgePropertyDef Prop;
		Prop.Name                = TEXT("Label");
		Prop.Type                = TEXT("FString");
		Prop.bEditAnywhere       = true;
		Prop.bBlueprintReadWrite = true;
		BP->StructProperties.Add(Prop);
	}

	// --- Step 1: Validate -----------------------------------------------
	TArray<FCodeForgeValidationResult> Errors = BP->Validate();

	int32 ErrorCount = 0;
	for (const FCodeForgeValidationResult& R : Errors)
	{
		if (R.Severity == ECodeForgeValidationSeverity::Error)
		{
			++ErrorCount;
		}
	}
	TestEqual(TEXT("Struct validation should report zero errors"), ErrorCount, 0);

	// --- Step 2: Generate -----------------------------------------------
	FCodeForgeGenerator Generator;
	Generator.SetTemplatePath(GetTemplatePath());

	FCodeForgeGeneratorResult Result = Generator.Generate(BP);
	TestTrue(TEXT("Struct generation must succeed"), Result.bSuccess);

	if (!Result.bSuccess)
	{
		AddError(FString::Printf(TEXT("Struct generation failed: %s"), *Result.ErrorMessage));
		return true;
	}

	// --- Step 3: Header content checks ----------------------------------
	const FString& H = Result.HeaderContent;
	TestTrue(TEXT("Header contains USTRUCT("),          H.Contains(TEXT("USTRUCT(")));
	TestTrue(TEXT("Header contains GENERATED_BODY()"),  H.Contains(TEXT("GENERATED_BODY()")));
	TestTrue(TEXT("Header contains Value"),             H.Contains(TEXT("Value")));
	TestTrue(TEXT("Header contains Label"),             H.Contains(TEXT("Label")));
	TestTrue(TEXT("Header contains BlueprintType"),     H.Contains(TEXT("BlueprintType")));

	// --- Step 4: Source content is NOT empty -----------------------------
	TestFalse(TEXT("Struct SourceContent must not be empty"), Result.SourceContent.IsEmpty());

	return true;
}

// ===========================================================================
// Test 3 — Enum generation
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCodeForgeEndToEndTest_EnumGeneration,
	"CodeForge.EndToEnd.EnumGeneration",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeEndToEndTest_EnumGeneration::RunTest(const FString& Parameters)
{
	// --- Build Blueprint ------------------------------------------------
	UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>(GetTransientPackage());
	BP->ClassName      = TEXT("ETestColor");
	BP->BlueprintKind  = ECodeForgeBlueprintKind::Enum;
	BP->bBlueprintType = true;

	// Entry 1 — Red
	{
		FCodeForgeEnumEntryDef Entry;
		Entry.Name              = TEXT("Red");
		Entry.DisplayName       = TEXT("Red Color");
		Entry.bHasExplicitValue = true;
		Entry.ExplicitValue     = 0;
		BP->EnumEntries.Add(Entry);
	}

	// Entry 2 — Green (no explicit value, no display name)
	{
		FCodeForgeEnumEntryDef Entry;
		Entry.Name              = TEXT("Green");
		Entry.DisplayName       = TEXT("");
		Entry.bHasExplicitValue = false;
		BP->EnumEntries.Add(Entry);
	}

	// Entry 3 — Blue
	{
		FCodeForgeEnumEntryDef Entry;
		Entry.Name              = TEXT("Blue");
		Entry.DisplayName       = TEXT("Blue Color");
		Entry.bHasExplicitValue = true;
		Entry.ExplicitValue     = 5;
		BP->EnumEntries.Add(Entry);
	}

	// Entry 4 — MAX (hidden)
	{
		FCodeForgeEnumEntryDef Entry;
		Entry.Name              = TEXT("MAX");
		Entry.bHidden           = true;
		Entry.bHasExplicitValue = false;
		BP->EnumEntries.Add(Entry);
	}

	// --- Step 1: Validate -----------------------------------------------
	TArray<FCodeForgeValidationResult> Errors = BP->Validate();

	int32 ErrorCount = 0;
	for (const FCodeForgeValidationResult& R : Errors)
	{
		if (R.Severity == ECodeForgeValidationSeverity::Error)
		{
			++ErrorCount;
		}
	}
	TestEqual(TEXT("Enum validation should report zero errors"), ErrorCount, 0);

	// --- Step 2: Generate -----------------------------------------------
	FCodeForgeGenerator Generator;
	Generator.SetTemplatePath(GetTemplatePath());

	FCodeForgeGeneratorResult Result = Generator.Generate(BP);
	TestTrue(TEXT("Enum generation must succeed"), Result.bSuccess);

	if (!Result.bSuccess)
	{
		AddError(FString::Printf(TEXT("Enum generation failed: %s"), *Result.ErrorMessage));
		return true;
	}

	// --- Step 3: Header content checks ----------------------------------
	const FString& H = Result.HeaderContent;
	TestTrue(TEXT("Header contains UENUM("),          H.Contains(TEXT("UENUM(")));
	TestTrue(TEXT("Header contains Red"),             H.Contains(TEXT("Red")));
	TestTrue(TEXT("Header contains Green"),           H.Contains(TEXT("Green")));
	TestTrue(TEXT("Header contains Blue"),            H.Contains(TEXT("Blue")));
	TestTrue(TEXT("Header contains MAX"),             H.Contains(TEXT("MAX")));
	TestTrue(TEXT("Header contains BlueprintType"),   H.Contains(TEXT("BlueprintType")));
	TestTrue(TEXT("Header contains Hidden"),          H.Contains(TEXT("Hidden")));

	// --- Step 4: Source content IS empty ---------------------------------
	TestTrue(TEXT("Enum SourceContent must be empty"), Result.SourceContent.IsEmpty());

	return true;
}

// ===========================================================================
// Test 4 — Validation catches errors and generation fails
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCodeForgeEndToEndTest_ValidationCatchesErrors,
	"CodeForge.EndToEnd.ValidationCatchesErrors",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeEndToEndTest_ValidationCatchesErrors::RunTest(const FString& Parameters)
{
	// --- Build an invalid Blueprint (empty ClassName) -------------------
	UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>(GetTransientPackage());
	BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
	BP->ClassName     = TEXT("");  // intentionally invalid

	// --- Step 1: Validate should catch the error ------------------------
	TArray<FCodeForgeValidationResult> Errors = BP->Validate();
	TestTrue(TEXT("Validation must report at least one issue"), Errors.Num() > 0);

	// --- Step 2: Generation should fail ---------------------------------
	FCodeForgeGenerator Generator;
	Generator.SetTemplatePath(GetTemplatePath());

	FCodeForgeGeneratorResult Result = Generator.Generate(BP);
	TestFalse(TEXT("Generation must fail for invalid blueprint"), Result.bSuccess);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS


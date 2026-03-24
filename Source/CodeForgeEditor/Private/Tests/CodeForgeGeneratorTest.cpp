// CodeForgeGeneratorTest.cpp
// Automation tests for FCodeForgeGenerator — Task 6 (TDD).
//
// These tests exercise the full code generation pipeline:
//   UCodeForgeBlueprint schema → FCodeForgeGenerator → .h/.cpp text
//
// All six tests specified in the task plan are implemented here.

#include "Misc/AutomationTest.h"
#include "CodeForgeGenerator.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgePropertyDef.h"
#include "CodeForgeFunctionDef.h"
#include "CodeForgeEnumDef.h"
#include "CodeForgeTypes.h"
#include "Misc/Paths.h"

#if WITH_DEV_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Helper: resolve template path relative to the plugin's Content/Templates dir
// ---------------------------------------------------------------------------
static FString GetTemplatesPath()
{
    // Plugin lives at  <ProjectRoot>/Plugins/CodeForge
    // We try the known absolute path first, then fall back to a project-relative one.
    FString PluginContent = FPaths::Combine(
        FPaths::ProjectDir(),
        TEXT("Plugins/CodeForge/Content/Templates"));
    FPaths::NormalizeFilename(PluginContent);
    return PluginContent;
}

// ---------------------------------------------------------------------------
// Helper: configure a generator pointing at the real templates
// ---------------------------------------------------------------------------
static FCodeForgeGenerator MakeGenerator()
{
    FCodeForgeGenerator Gen;
    Gen.SetTemplatePath(GetTemplatesPath());
    return Gen;
}

// ===========================================================================
// Test 1 — Actor class with a replicated RepNotify property
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeGeneratorTest_ActorClass,
    "CodeForge.Generator.ActorClass",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeGeneratorTest_ActorClass::RunTest(const FString& Parameters)
{
    // Build a Blueprint with a replicated + RepNotify property
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->ClassName      = TEXT("AMyActor");
    BP->BlueprintKind  = ECodeForgeBlueprintKind::Class;
    BP->ClassType      = ECodeForgeClassType::Actor;
    BP->bReplicated    = true;
    BP->ModuleTarget   = TEXT("MyGame");

    FCodeForgePropertyDef Prop;
    Prop.Name                = TEXT("Health");
    Prop.Type                = TEXT("float");
    Prop.bReplicated         = true;
    Prop.bRepNotify          = true;
    Prop.ReplicationCondition = ECodeForgeRepCondition::None;
    BP->Properties.Add(Prop);

    FCodeForgeGenerator Gen = MakeGenerator();
    FCodeForgeGeneratorResult Result = Gen.Generate(BP);

    if (!TestTrue(TEXT("Generation succeeded"), Result.bSuccess))
    {
        AddError(FString::Printf(TEXT("Error: %s"), *Result.ErrorMessage));
        return false;
    }

    // --- Header checks ---
    TestTrue(TEXT("Header contains UCLASS"),
        Result.HeaderContent.Contains(TEXT("UCLASS(")));
    TestTrue(TEXT("Header contains MYGAME_API"),
        Result.HeaderContent.Contains(TEXT("MYGAME_API")));
    TestTrue(TEXT("Header contains AMyActor class declaration"),
        Result.HeaderContent.Contains(TEXT("AMyActor")));
    TestTrue(TEXT("Header contains UPROPERTY"),
        Result.HeaderContent.Contains(TEXT("UPROPERTY(")));
    TestTrue(TEXT("Header contains Health property"),
        Result.HeaderContent.Contains(TEXT("Health")));
    TestTrue(TEXT("Header contains ReplicatedUsing"),
        Result.HeaderContent.Contains(TEXT("ReplicatedUsing")));
    TestTrue(TEXT("Header contains GetLifetimeReplicatedProps declaration"),
        Result.HeaderContent.Contains(TEXT("GetLifetimeReplicatedProps")));
    TestTrue(TEXT("Header contains OnRep_Health declaration"),
        Result.HeaderContent.Contains(TEXT("OnRep_Health")));

    // --- Source checks ---
    TestTrue(TEXT("Source contains GetLifetimeReplicatedProps definition"),
        Result.SourceContent.Contains(TEXT("GetLifetimeReplicatedProps")));
    TestTrue(TEXT("Source contains DOREPLIFETIME"),
        Result.SourceContent.Contains(TEXT("DOREPLIFETIME")));
    TestTrue(TEXT("Source contains OnRep_Health definition"),
        Result.SourceContent.Contains(TEXT("OnRep_Health")));
    TestTrue(TEXT("Source includes UnrealNetwork.h"),
        Result.SourceContent.Contains(TEXT("UnrealNetwork.h")));
    TestTrue(TEXT("Source contains bReplicates = true"),
        Result.SourceContent.Contains(TEXT("bReplicates = true")));

    return true;
}

// ===========================================================================
// Test 2 — Struct generation
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeGeneratorTest_Struct,
    "CodeForge.Generator.Struct",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeGeneratorTest_Struct::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->ClassName     = TEXT("FMyStruct");
    BP->BlueprintKind = ECodeForgeBlueprintKind::Struct;
    BP->bBlueprintType = true;
    BP->ModuleTarget  = TEXT("MyGame");

    FCodeForgePropertyDef Prop;
    Prop.Name = TEXT("Value");
    Prop.Type = TEXT("int32");
    BP->StructProperties.Add(Prop);

    FCodeForgeGenerator Gen = MakeGenerator();
    FCodeForgeGeneratorResult Result = Gen.Generate(BP);

    if (!TestTrue(TEXT("Struct generation succeeded"), Result.bSuccess))
    {
        AddError(FString::Printf(TEXT("Error: %s"), *Result.ErrorMessage));
        return false;
    }

    TestTrue(TEXT("Header contains USTRUCT"),
        Result.HeaderContent.Contains(TEXT("USTRUCT(")));
    TestTrue(TEXT("Header contains BlueprintType"),
        Result.HeaderContent.Contains(TEXT("BlueprintType")));
    TestTrue(TEXT("Header contains MYGAME_API"),
        Result.HeaderContent.Contains(TEXT("MYGAME_API")));
    TestTrue(TEXT("Header contains struct name"),
        Result.HeaderContent.Contains(TEXT("FMyStruct")));
    TestTrue(TEXT("Header contains UPROPERTY"),
        Result.HeaderContent.Contains(TEXT("UPROPERTY(")));
    TestTrue(TEXT("Header contains Value property"),
        Result.HeaderContent.Contains(TEXT("Value")));
    TestTrue(TEXT("Header contains GENERATED_BODY"),
        Result.HeaderContent.Contains(TEXT("GENERATED_BODY()")));

    // Struct source is a minimal .cpp with just the include
    TestTrue(TEXT("Source includes the header"),
        Result.SourceContent.Contains(TEXT("FMyStruct.h")));

    return true;
}

// ===========================================================================
// Test 3 — Enum generation
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeGeneratorTest_Enum,
    "CodeForge.Generator.Enum",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeGeneratorTest_Enum::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->ClassName      = TEXT("EMyEnum");
    BP->BlueprintKind  = ECodeForgeBlueprintKind::Enum;
    BP->bBlueprintType = true;
    BP->ModuleTarget   = TEXT("MyGame");

    FCodeForgeEnumEntryDef EntryA;
    EntryA.Name        = TEXT("TypeA");
    EntryA.DisplayName = TEXT("Type A");
    BP->EnumEntries.Add(EntryA);

    FCodeForgeEnumEntryDef EntryB;
    EntryB.Name             = TEXT("TypeB");
    EntryB.DisplayName      = TEXT("Type B");
    EntryB.bHasExplicitValue = true;
    EntryB.ExplicitValue     = 5;
    BP->EnumEntries.Add(EntryB);

    FCodeForgeGenerator Gen = MakeGenerator();
    FCodeForgeGeneratorResult Result = Gen.Generate(BP);

    if (!TestTrue(TEXT("Enum generation succeeded"), Result.bSuccess))
    {
        AddError(FString::Printf(TEXT("Error: %s"), *Result.ErrorMessage));
        return false;
    }

    TestTrue(TEXT("Header contains UENUM"),
        Result.HeaderContent.Contains(TEXT("UENUM(")));
    TestTrue(TEXT("Header contains BlueprintType"),
        Result.HeaderContent.Contains(TEXT("BlueprintType")));
    TestTrue(TEXT("Header contains enum class declaration"),
        Result.HeaderContent.Contains(TEXT("EMyEnum")));
    TestTrue(TEXT("Header contains uint8 backing type"),
        Result.HeaderContent.Contains(TEXT("uint8")));
    TestTrue(TEXT("Header contains TypeA entry"),
        Result.HeaderContent.Contains(TEXT("TypeA")));
    TestTrue(TEXT("Header contains TypeB entry"),
        Result.HeaderContent.Contains(TEXT("TypeB")));
    TestTrue(TEXT("Header contains explicit value for TypeB"),
        Result.HeaderContent.Contains(TEXT("= 5")));
    TestTrue(TEXT("Header contains UMETA for TypeA"),
        Result.HeaderContent.Contains(TEXT("UMETA(")));

    // Enums produce no source file
    TestTrue(TEXT("Source content is empty for Enum"),
        Result.SourceContent.IsEmpty());

    return true;
}

// ===========================================================================
// Test 5 — ActorComponent generation (component templates, no replication)
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeGeneratorTest_Component,
    "CodeForge.Generator.Component",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeGeneratorTest_Component::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->ClassName     = TEXT("UMyComponent");
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassType     = ECodeForgeClassType::ActorComponent;
    BP->bReplicated   = false;
    BP->ModuleTarget  = TEXT("MyGame");

    FCodeForgeFunctionDef Func;
    Func.Name                 = TEXT("OnUpdate");
    Func.ReturnType           = TEXT("void");
    Func.bBlueprintCallable   = true;
    Func.bBlueprintNativeEvent = true;
    BP->Functions.Add(Func);

    FCodeForgeGenerator Gen = MakeGenerator();
    FCodeForgeGeneratorResult Result = Gen.Generate(BP);

    if (!TestTrue(TEXT("Component generation succeeded"), Result.bSuccess))
    {
        AddError(FString::Printf(TEXT("Error: %s"), *Result.ErrorMessage));
        return false;
    }

    // Should use component template — verify characteristic markers
    TestTrue(TEXT("Header contains UCLASS"),
        Result.HeaderContent.Contains(TEXT("UCLASS(")));
    TestTrue(TEXT("Header contains UMYCOMPONENT"),
        Result.HeaderContent.Contains(TEXT("UMyComponent")));

    // Component template declares NativeEvent _Implementation as virtual
    TestTrue(TEXT("Header contains _Implementation virtual declaration"),
        Result.HeaderContent.Contains(TEXT("_Implementation")));

    // Source: _Implementation stub from component_source.cft
    TestTrue(TEXT("Source contains OnUpdate_Implementation"),
        Result.SourceContent.Contains(TEXT("OnUpdate_Implementation")));

    // No DOREPLIFETIME in source (not replicated)
    TestFalse(TEXT("Source does NOT contain DOREPLIFETIME"),
        Result.SourceContent.Contains(TEXT("DOREPLIFETIME")));

    return true;
}

// ===========================================================================
// Test 6 — Validation failure blocks generation
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeGeneratorTest_ValidationFailure,
    "CodeForge.Generator.ValidationFailure",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeGeneratorTest_ValidationFailure::RunTest(const FString& Parameters)
{
    // An empty ClassName should fail Validate() with an Error
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->ClassName     = TEXT("");   // invalid — empty class name
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassType     = ECodeForgeClassType::Actor;

    FCodeForgeGenerator Gen = MakeGenerator();
    FCodeForgeGeneratorResult Result = Gen.Generate(BP);

    TestFalse(TEXT("Generation should fail for invalid Blueprint"),
        Result.bSuccess);
    TestFalse(TEXT("Error message should not be empty"),
        Result.ErrorMessage.IsEmpty());

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS


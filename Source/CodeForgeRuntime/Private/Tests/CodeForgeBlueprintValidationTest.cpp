// CodeForgeBlueprintValidationTest.cpp
// Automation tests for UCodeForgeBlueprint::Validate() and ApplyAutoFix() — Task 4.
// One test per validation rule plus auto-fix and positive tests.

#include "Misc/AutomationTest.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgePropertyDef.h"
#include "CodeForgeFunctionDef.h"
#include "CodeForgeTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Helper: build a minimal valid Actor blueprint (used by several tests)
// ---------------------------------------------------------------------------
static UCodeForgeBlueprint* MakeValidActorBlueprint()
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("AMyActor");
    BP->ClassType     = ECodeForgeClassType::Actor;

    FCodeForgePropertyDef Prop;
    Prop.Name = TEXT("Health");
    Prop.Type = TEXT("float");
    BP->Properties.Add(Prop);

    return BP;
}

// ===========================================================================
// Rule 1 — Duplicate property/function names within the same class
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule01_DuplicatePropertyNames,
    "CodeForge.Validation.Rule01.DuplicatePropertyNames",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule01_DuplicatePropertyNames::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("AMyActor");
    BP->ClassType     = ECodeForgeClassType::Actor;

    FCodeForgePropertyDef P1; P1.Name = TEXT("Health"); P1.Type = TEXT("float");
    FCodeForgePropertyDef P2; P2.Name = TEXT("Health"); P2.Type = TEXT("float"); // duplicate
    BP->Properties.Add(P1);
    BP->Properties.Add(P2);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    bool bHasError = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Error
            && R.Message.Contains(TEXT("Duplicate"));
    });
    TestTrue(TEXT("Should report duplicate property name error"), bHasError);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule01_DuplicateFunctionNames,
    "CodeForge.Validation.Rule01.DuplicateFunctionNames",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule01_DuplicateFunctionNames::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("AMyActor");
    BP->ClassType     = ECodeForgeClassType::Actor;

    FCodeForgeFunctionDef F1; F1.Name = TEXT("Fire");
    FCodeForgeFunctionDef F2; F2.Name = TEXT("Fire"); // duplicate
    BP->Functions.Add(F1);
    BP->Functions.Add(F2);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    bool bHasError = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Error
            && R.Message.Contains(TEXT("Duplicate"));
    });
    TestTrue(TEXT("Should report duplicate function name error"), bHasError);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule01_DuplicateAcrossPropAndFunc,
    "CodeForge.Validation.Rule01.DuplicateAcrossPropertyAndFunction",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule01_DuplicateAcrossPropAndFunc::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("AMyActor");
    BP->ClassType     = ECodeForgeClassType::Actor;

    FCodeForgePropertyDef P; P.Name = TEXT("Fire"); P.Type = TEXT("float");
    FCodeForgeFunctionDef F; F.Name = TEXT("Fire"); // same name as property
    BP->Properties.Add(P);
    BP->Functions.Add(F);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    bool bHasError = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Error
            && R.Message.Contains(TEXT("Duplicate"));
    });
    TestTrue(TEXT("Should report duplicate name across property and function"), bHasError);
    return true;
}

// ===========================================================================
// Rule 2 — Invalid specifier combo (bBlueprintReadWrite + bBlueprintReadOnly both true)
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule02_InvalidSpecifierCombo,
    "CodeForge.Validation.Rule02.InvalidSpecifierCombo",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule02_InvalidSpecifierCombo::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("AMyActor");
    BP->ClassType     = ECodeForgeClassType::Actor;

    FCodeForgePropertyDef Prop;
    Prop.Name = TEXT("Health");
    Prop.Type = TEXT("float");
    Prop.bBlueprintReadWrite = true;
    Prop.bBlueprintReadOnly  = true; // invalid combo
    BP->Properties.Add(Prop);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    bool bHasError = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Error
            && (R.Message.Contains(TEXT("BlueprintReadWrite")) || R.Message.Contains(TEXT("BlueprintReadOnly")));
    });
    TestTrue(TEXT("Should report invalid specifier combo error"), bHasError);
    return true;
}

// ===========================================================================
// Rule 4 — Missing parent class (ClassName is empty)
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule04_MissingClassName,
    "CodeForge.Validation.Rule04.MissingClassName",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule04_MissingClassName::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT(""); // empty — invalid

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    bool bHasError = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Error
            && R.FieldName == TEXT("ClassName");
    });
    TestTrue(TEXT("Should report missing class name error"), bHasError);
    return true;
}

// ===========================================================================
// Rule 5 — Reserved name collision ("Class", "Super", "StaticClass")
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule05_ReservedNameClass,
    "CodeForge.Validation.Rule05.ReservedNameClass",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule05_ReservedNameClass::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("AMyActor");
    BP->ClassType     = ECodeForgeClassType::Actor;

    FCodeForgePropertyDef Prop;
    Prop.Name = TEXT("Class"); // reserved
    Prop.Type = TEXT("FString");
    BP->Properties.Add(Prop);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    bool bHasError = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Error
            && R.Message.Contains(TEXT("Class"));
    });
    TestTrue(TEXT("Should report reserved name 'Class' error"), bHasError);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule05_ReservedNameSuper,
    "CodeForge.Validation.Rule05.ReservedNameSuper",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule05_ReservedNameSuper::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("AMyActor");
    BP->ClassType     = ECodeForgeClassType::Actor;

    FCodeForgeFunctionDef Func;
    Func.Name = TEXT("Super"); // reserved
    BP->Functions.Add(Func);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    bool bHasError = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Error
            && R.Message.Contains(TEXT("Super"));
    });
    TestTrue(TEXT("Should report reserved name 'Super' error"), bHasError);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule05_ReservedNameStaticClass,
    "CodeForge.Validation.Rule05.ReservedNameStaticClass",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule05_ReservedNameStaticClass::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("AMyActor");
    BP->ClassType     = ECodeForgeClassType::Actor;

    FCodeForgePropertyDef Prop;
    Prop.Name = TEXT("StaticClass"); // reserved
    Prop.Type = TEXT("UClass*");
    BP->Properties.Add(Prop);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    bool bHasError = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Error
            && R.Message.Contains(TEXT("StaticClass"));
    });
    TestTrue(TEXT("Should report reserved name 'StaticClass' error"), bHasError);
    return true;
}

// ===========================================================================
// Rule 6 — C++ keyword as name
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule06_CppKeywordProperty,
    "CodeForge.Validation.Rule06.CppKeywordAsPropertyName",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule06_CppKeywordProperty::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("AMyActor");
    BP->ClassType     = ECodeForgeClassType::Actor;

    FCodeForgePropertyDef Prop;
    Prop.Name = TEXT("delete"); // C++ keyword
    Prop.Type = TEXT("float");
    BP->Properties.Add(Prop);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    bool bHasError = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Error
            && R.Message.Contains(TEXT("delete"));
    });
    TestTrue(TEXT("Should report C++ keyword 'delete' error"), bHasError);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule06_CppKeywordFunction,
    "CodeForge.Validation.Rule06.CppKeywordAsFunctionName",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule06_CppKeywordFunction::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("AMyActor");
    BP->ClassType     = ECodeForgeClassType::Actor;

    FCodeForgeFunctionDef Func;
    Func.Name = TEXT("return"); // C++ keyword
    BP->Functions.Add(Func);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    bool bHasError = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Error
            && R.Message.Contains(TEXT("return"));
    });
    TestTrue(TEXT("Should report C++ keyword 'return' error for function"), bHasError);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule06_MultipleCppKeywords,
    "CodeForge.Validation.Rule06.MultipleCppKeywords",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule06_MultipleCppKeywords::RunTest(const FString& Parameters)
{
    // Spot-check several keywords to ensure the full set is covered
    const TArray<FString> KeywordsToTest = {
        TEXT("static"), TEXT("virtual"), TEXT("const"), TEXT("new"),
        TEXT("namespace"), TEXT("template"), TEXT("auto"), TEXT("try"),
        TEXT("catch"), TEXT("throw")
    };

    for (const FString& Keyword : KeywordsToTest)
    {
        UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
        BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
        BP->ClassName     = TEXT("AMyActor");
        BP->ClassType     = ECodeForgeClassType::Actor;

        FCodeForgePropertyDef Prop;
        Prop.Name = Keyword;
        Prop.Type = TEXT("float");
        BP->Properties.Add(Prop);

        TArray<FCodeForgeValidationResult> Results = BP->Validate();

        bool bHasError = Results.ContainsByPredicate([&Keyword](const FCodeForgeValidationResult& R)
        {
            return R.Severity == ECodeForgeValidationSeverity::Error
                && R.Message.Contains(Keyword);
        });
        TestTrue(*FString::Printf(TEXT("Should report error for C++ keyword '%s'"), *Keyword), bHasError);
    }

    return true;
}

// ===========================================================================
// Rule 8 — RepNotify without Replicated
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule08_RepNotifyWithoutReplicated,
    "CodeForge.Validation.Rule08.RepNotifyWithoutReplicated",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule08_RepNotifyWithoutReplicated::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("AMyActor");
    BP->ClassType     = ECodeForgeClassType::Actor;

    FCodeForgePropertyDef Prop;
    Prop.Name       = TEXT("Health");
    Prop.Type       = TEXT("float");
    Prop.bRepNotify = true;
    Prop.bReplicated = false; // invalid: RepNotify without Replicated
    BP->Properties.Add(Prop);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    bool bHasError = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Error
            && R.Message.Contains(TEXT("RepNotify"))
            && !R.AutoFixId.IsEmpty();
    });
    TestTrue(TEXT("Should report RepNotify-without-Replicated error with AutoFixId"), bHasError);

    // Verify the AutoFixId has the correct format
    FString ExpectedAutoFixId = TEXT("EnableReplicated:Health");
    bool bHasCorrectAutoFixId = Results.ContainsByPredicate([&ExpectedAutoFixId](const FCodeForgeValidationResult& R)
    {
        return R.AutoFixId == ExpectedAutoFixId;
    });
    TestTrue(TEXT("AutoFixId should be 'EnableReplicated:Health'"), bHasCorrectAutoFixId);

    return true;
}

// ===========================================================================
// Rule 9 — Bool not prefixed with 'b'
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule09_BoolMissingPrefix,
    "CodeForge.Validation.Rule09.BoolMissingBPrefix",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule09_BoolMissingPrefix::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("AMyActor");
    BP->ClassType     = ECodeForgeClassType::Actor;

    FCodeForgePropertyDef Prop;
    Prop.Name = TEXT("isAlive"); // should be bIsAlive
    Prop.Type = TEXT("bool");
    BP->Properties.Add(Prop);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    bool bHasWarning = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Warning
            && R.Message.Contains(TEXT("bool"))
            && !R.AutoFixId.IsEmpty();
    });
    TestTrue(TEXT("Should warn about bool missing 'b' prefix"), bHasWarning);

    FString ExpectedAutoFixId = TEXT("FixBoolPrefix:isAlive");
    bool bHasCorrectAutoFixId = Results.ContainsByPredicate([&ExpectedAutoFixId](const FCodeForgeValidationResult& R)
    {
        return R.AutoFixId == ExpectedAutoFixId;
    });
    TestTrue(TEXT("AutoFixId should be 'FixBoolPrefix:isAlive'"), bHasCorrectAutoFixId);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule09_BoolLowercaseSecondChar,
    "CodeForge.Validation.Rule09.BoolLowercaseAfterB",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule09_BoolLowercaseSecondChar::RunTest(const FString& Parameters)
{
    // "balive" starts with 'b' but second char is lowercase — still invalid
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("AMyActor");
    BP->ClassType     = ECodeForgeClassType::Actor;

    FCodeForgePropertyDef Prop;
    Prop.Name = TEXT("balive"); // 'b' followed by lowercase — invalid
    Prop.Type = TEXT("bool");
    BP->Properties.Add(Prop);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    bool bHasWarning = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Warning
            && R.Message.Contains(TEXT("bool"));
    });
    TestTrue(TEXT("Should warn about 'balive' (b + lowercase) bool property"), bHasWarning);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule09_BoolCorrectPrefix_NoWarning,
    "CodeForge.Validation.Rule09.BoolCorrectPrefixNoWarning",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule09_BoolCorrectPrefix_NoWarning::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("AMyActor");
    BP->ClassType     = ECodeForgeClassType::Actor;

    FCodeForgePropertyDef Prop;
    Prop.Name = TEXT("bIsAlive"); // correct
    Prop.Type = TEXT("bool");
    BP->Properties.Add(Prop);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    bool bHasBoolWarning = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Warning
            && R.Message.Contains(TEXT("bool"));
    });
    TestFalse(TEXT("Should NOT warn about correctly-prefixed bool 'bIsAlive'"), bHasBoolWarning);
    return true;
}

// ===========================================================================
// Rule 10 — Empty class
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule10_EmptyClass,
    "CodeForge.Validation.Rule10.EmptyClass",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule10_EmptyClass::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("AMyActor");
    BP->ClassType     = ECodeForgeClassType::Actor;
    // No properties or functions added

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    bool bHasWarning = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Warning
            && (R.Message.Contains(TEXT("no properties")) || R.Message.Contains(TEXT("empty"))
                || R.Message.Contains(TEXT("no properties, functions")));
    });
    TestTrue(TEXT("Should warn about empty class"), bHasWarning);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule10_NonEmptyClass_NoEmptyWarning,
    "CodeForge.Validation.Rule10.NonEmptyClassNoWarning",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule10_NonEmptyClass_NoEmptyWarning::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = MakeValidActorBlueprint();
    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    // Should have no errors or warnings about empty class
    bool bHasEmptyWarning = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Warning
            && R.FieldName == TEXT("Class");
    });
    TestFalse(TEXT("Non-empty class should not trigger empty-class warning"), bHasEmptyWarning);
    return true;
}

// ===========================================================================
// Rule 11 — Replicated property on non-Actor class
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule11_ReplicatedOnNonActor,
    "CodeForge.Validation.Rule11.ReplicatedPropertyOnNonActorClass",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule11_ReplicatedOnNonActor::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("UMyObject");
    BP->ClassType     = ECodeForgeClassType::Object; // NOT Actor-derived

    FCodeForgePropertyDef Prop;
    Prop.Name        = TEXT("Health");
    Prop.Type        = TEXT("float");
    Prop.bReplicated = true;
    BP->Properties.Add(Prop);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    bool bHasWarning = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Warning
            && R.Message.Contains(TEXT("replicated"));
    });
    TestTrue(TEXT("Should warn about replicated property on non-Actor class"), bHasWarning);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule11_ReplicatedOnActorClass_NoWarning,
    "CodeForge.Validation.Rule11.ReplicatedPropertyOnActorClassOK",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule11_ReplicatedOnActorClass_NoWarning::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("AMyActor");
    BP->ClassType     = ECodeForgeClassType::Actor;

    FCodeForgePropertyDef Prop;
    Prop.Name        = TEXT("Health");
    Prop.Type        = TEXT("float");
    Prop.bReplicated = true;
    BP->Properties.Add(Prop);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    bool bHasReplicationWarning = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Warning
            && R.Message.Contains(TEXT("replicated"));
    });
    TestFalse(TEXT("Should NOT warn about replicated property on Actor-derived class"), bHasReplicationWarning);
    return true;
}

// ===========================================================================
// Rule 12 — Replication specifiers on Struct property
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule12_ReplicationOnStructProperty,
    "CodeForge.Validation.Rule12.ReplicationFlagsOnStructProperty",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule12_ReplicationOnStructProperty::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Struct;
    BP->ClassName     = TEXT("FMyStruct");

    FCodeForgePropertyDef Prop;
    Prop.Name        = TEXT("Health");
    Prop.Type        = TEXT("float");
    Prop.bReplicated = true; // invalid on a struct
    BP->StructProperties.Add(Prop);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    bool bHasError = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Error
            && R.Message.Contains(TEXT("replication"))
            && !R.AutoFixId.IsEmpty();
    });
    TestTrue(TEXT("Should report error for replicated struct property with AutoFixId"), bHasError);

    FString ExpectedAutoFixId = TEXT("RemoveReplicationFlags:Health");
    bool bHasCorrectAutoFixId = Results.ContainsByPredicate([&ExpectedAutoFixId](const FCodeForgeValidationResult& R)
    {
        return R.AutoFixId == ExpectedAutoFixId;
    });
    TestTrue(TEXT("AutoFixId should be 'RemoveReplicationFlags:Health'"), bHasCorrectAutoFixId);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Rule12_NoReplicationOnStruct_NoError,
    "CodeForge.Validation.Rule12.NoReplicationOnStructIsValid",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Rule12_NoReplicationOnStruct_NoError::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Struct;
    BP->ClassName     = TEXT("FMyStruct");

    FCodeForgePropertyDef Prop;
    Prop.Name        = TEXT("Health");
    Prop.Type        = TEXT("float");
    Prop.bReplicated = false; // valid
    BP->StructProperties.Add(Prop);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    bool bHasError = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Error
            && R.Message.Contains(TEXT("replication"));
    });
    TestFalse(TEXT("Should NOT report error for non-replicated struct property"), bHasError);
    return true;
}

// ===========================================================================
// Auto-fix tests
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_AutoFix_EnableReplicated,
    "CodeForge.Validation.AutoFix.EnableReplicated",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_AutoFix_EnableReplicated::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("AMyActor");
    BP->ClassType     = ECodeForgeClassType::Actor;

    FCodeForgePropertyDef Prop;
    Prop.Name        = TEXT("Health");
    Prop.Type        = TEXT("float");
    Prop.bRepNotify  = true;
    Prop.bReplicated = false;
    BP->Properties.Add(Prop);

    // Validate to get the AutoFixId
    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    FString AutoFixId;
    for (const FCodeForgeValidationResult& R : Results)
    {
        if (R.AutoFixId.StartsWith(TEXT("EnableReplicated:")))
        {
            AutoFixId = R.AutoFixId;
            break;
        }
    }
    TestFalse(TEXT("AutoFixId should be found"), AutoFixId.IsEmpty());

    // Apply the fix
    BP->ApplyAutoFix(AutoFixId);

    // Verify the property is now replicated
    TestTrue(TEXT("Property bReplicated should be true after auto-fix"), BP->Properties[0].bReplicated);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_AutoFix_FixBoolPrefix,
    "CodeForge.Validation.AutoFix.FixBoolPrefix",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_AutoFix_FixBoolPrefix::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("AMyActor");
    BP->ClassType     = ECodeForgeClassType::Actor;

    FCodeForgePropertyDef Prop;
    Prop.Name = TEXT("isAlive");
    Prop.Type = TEXT("bool");
    BP->Properties.Add(Prop);

    // Apply fix directly by known AutoFixId
    BP->ApplyAutoFix(TEXT("FixBoolPrefix:isAlive"));

    // Verify the name is now "bIsAlive"
    TestEqual(TEXT("Property name should be 'bIsAlive' after auto-fix"),
        BP->Properties[0].Name, FString(TEXT("bIsAlive")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_AutoFix_FixBoolPrefix_StructProperty,
    "CodeForge.Validation.AutoFix.FixBoolPrefixOnStructProperty",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_AutoFix_FixBoolPrefix_StructProperty::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Struct;
    BP->ClassName     = TEXT("FMyStruct");

    FCodeForgePropertyDef Prop;
    Prop.Name = TEXT("active");
    Prop.Type = TEXT("bool");
    BP->StructProperties.Add(Prop);

    BP->ApplyAutoFix(TEXT("FixBoolPrefix:active"));

    TestEqual(TEXT("Struct property name should be 'bActive' after auto-fix"),
        BP->StructProperties[0].Name, FString(TEXT("bActive")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_AutoFix_RemoveReplicationFlags,
    "CodeForge.Validation.AutoFix.RemoveReplicationFlags",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_AutoFix_RemoveReplicationFlags::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Struct;
    BP->ClassName     = TEXT("FMyStruct");

    FCodeForgePropertyDef Prop;
    Prop.Name        = TEXT("Health");
    Prop.Type        = TEXT("float");
    Prop.bReplicated = true;
    Prop.bRepNotify  = true;
    BP->StructProperties.Add(Prop);

    // Validate to confirm AutoFixId is present
    TArray<FCodeForgeValidationResult> Results = BP->Validate();
    bool bHasAutoFix = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.AutoFixId == TEXT("RemoveReplicationFlags:Health");
    });
    TestTrue(TEXT("Validation should provide RemoveReplicationFlags AutoFixId"), bHasAutoFix);

    // Apply the fix
    BP->ApplyAutoFix(TEXT("RemoveReplicationFlags:Health"));

    TestFalse(TEXT("bReplicated should be false after auto-fix"), BP->StructProperties[0].bReplicated);
    TestFalse(TEXT("bRepNotify should be false after auto-fix"),  BP->StructProperties[0].bRepNotify);
    return true;
}

// ===========================================================================
// Positive test — valid blueprint produces no errors
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_ValidBlueprint_NoErrors,
    "CodeForge.Validation.ValidBlueprint.NoErrors",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_ValidBlueprint_NoErrors::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("AMyGameActor");
    BP->ClassType     = ECodeForgeClassType::Actor;

    // Valid property: float, replicated on an Actor
    FCodeForgePropertyDef P1;
    P1.Name        = TEXT("Health");
    P1.Type        = TEXT("float");
    P1.bReplicated = true;
    P1.bRepNotify  = false;
    BP->Properties.Add(P1);

    // Valid bool property: prefixed correctly
    FCodeForgePropertyDef P2;
    P2.Name = TEXT("bIsAlive");
    P2.Type = TEXT("bool");
    BP->Properties.Add(P2);

    // Valid function
    FCodeForgeFunctionDef Func;
    Func.Name             = TEXT("Fire");
    Func.bBlueprintCallable = true;
    BP->Functions.Add(Func);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    int32 ErrorCount = 0;
    for (const FCodeForgeValidationResult& R : Results)
    {
        if (R.Severity == ECodeForgeValidationSeverity::Error)
        {
            ++ErrorCount;
        }
    }
    TestEqual(TEXT("Valid blueprint should have zero errors"), ErrorCount, 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_ValidStruct_NoErrors,
    "CodeForge.Validation.ValidBlueprint.ValidStructNoErrors",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_ValidStruct_NoErrors::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Struct;
    BP->ClassName     = TEXT("FMyData");

    FCodeForgePropertyDef P1;
    P1.Name        = TEXT("Score");
    P1.Type        = TEXT("int32");
    P1.bReplicated = false;
    BP->StructProperties.Add(P1);

    FCodeForgePropertyDef P2;
    P2.Name = TEXT("bIsValid");
    P2.Type = TEXT("bool");
    BP->StructProperties.Add(P2);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    int32 ErrorCount = 0;
    for (const FCodeForgeValidationResult& R : Results)
    {
        if (R.Severity == ECodeForgeValidationSeverity::Error)
        {
            ++ErrorCount;
        }
    }
    TestEqual(TEXT("Valid struct should have zero errors"), ErrorCount, 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_ValidEnum_NoErrors,
    "CodeForge.Validation.ValidBlueprint.ValidEnumNoErrors",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_ValidEnum_NoErrors::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Enum;
    BP->ClassName     = TEXT("EMyState");

    FCodeForgeEnumEntryDef Entry;
    Entry.Name = TEXT("Active");
    BP->EnumEntries.Add(Entry);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    int32 ErrorCount = 0;
    for (const FCodeForgeValidationResult& R : Results)
    {
        if (R.Severity == ECodeForgeValidationSeverity::Error)
        {
            ++ErrorCount;
        }
    }
    TestEqual(TEXT("Valid enum should have zero errors"), ErrorCount, 0);
    return true;
}

// ===========================================================================
// Edge cases
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Edge_StructDuplicatePropertyNames,
    "CodeForge.Validation.Edge.StructDuplicatePropertyNames",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Edge_StructDuplicatePropertyNames::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Struct;
    BP->ClassName     = TEXT("FMyStruct");

    FCodeForgePropertyDef P1; P1.Name = TEXT("Value"); P1.Type = TEXT("float");
    FCodeForgePropertyDef P2; P2.Name = TEXT("Value"); P2.Type = TEXT("int32"); // duplicate
    BP->StructProperties.Add(P1);
    BP->StructProperties.Add(P2);

    TArray<FCodeForgeValidationResult> Results = BP->Validate();

    bool bHasError = Results.ContainsByPredicate([](const FCodeForgeValidationResult& R)
    {
        return R.Severity == ECodeForgeValidationSeverity::Error
            && R.Message.Contains(TEXT("Duplicate"));
    });
    TestTrue(TEXT("Should report duplicate struct property name error"), bHasError);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidate_Edge_AutoFix_EnableReplicated_OnlyAffectsNamedProperty,
    "CodeForge.Validation.AutoFix.EnableReplicatedOnlyAffectsNamedProperty",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FValidate_Edge_AutoFix_EnableReplicated_OnlyAffectsNamedProperty::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
    BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
    BP->ClassName     = TEXT("AMyActor");
    BP->ClassType     = ECodeForgeClassType::Actor;

    FCodeForgePropertyDef P1; P1.Name = TEXT("Health"); P1.Type = TEXT("float"); P1.bRepNotify = true; P1.bReplicated = false;
    FCodeForgePropertyDef P2; P2.Name = TEXT("Ammo");   P2.Type = TEXT("int32"); P2.bReplicated = false;
    BP->Properties.Add(P1);
    BP->Properties.Add(P2);

    BP->ApplyAutoFix(TEXT("EnableReplicated:Health"));

    TestTrue (TEXT("Health.bReplicated should be true"),  BP->Properties[0].bReplicated);
    TestFalse(TEXT("Ammo.bReplicated should remain false"), BP->Properties[1].bReplicated);
    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS



// Copyright (c) 2026 GregOrigin. All Rights Reserved.

// CodeForgeTemplateEngineTest.cpp
// Automation tests for FCodeForgeTemplateEngine â€” written TDD-style (tests first).

#include "Misc/AutomationTest.h"
#include "CodeForgeTemplateEngine.h"

#if WITH_DEV_AUTOMATION_TESTS

// ============================================================
//  Test 1: Variable Substitution
// ============================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTemplateTest_VariableSubstitution,
    "CodeForge.TemplateEngine.VariableSubstitution",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTemplateTest_VariableSubstitution::RunTest(const FString& Parameters)
{
    FCodeForgeTemplateEngine Engine;
    FCodeForgeTemplateContext Context;
    Context.Variables.Add(TEXT("ClassName"),   TEXT("AMyActor"));
    Context.Variables.Add(TEXT("ParentClass"), TEXT("AActor"));

    FCodeForgeTemplateResult Result = Engine.Process(
        TEXT("class {{ClassName}} : public {{ParentClass}} {}"), Context);

    TestTrue(TEXT("bSuccess"), Result.bSuccess);
    TestEqual(TEXT("Output"), Result.Output,
        FString(TEXT("class AMyActor : public AActor {}")));
    return true;
}

// ============================================================
//  Test 2: Multiple Variables Same Line
// ============================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTemplateTest_MultipleVariablesSameLine,
    "CodeForge.TemplateEngine.MultipleVariablesSameLine",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTemplateTest_MultipleVariablesSameLine::RunTest(const FString& Parameters)
{
    FCodeForgeTemplateEngine Engine;
    FCodeForgeTemplateContext Context;
    Context.Variables.Add(TEXT("ReturnType"),   TEXT("float"));
    Context.Variables.Add(TEXT("FunctionName"), TEXT("GetHealth"));
    Context.Variables.Add(TEXT("Params"),       TEXT(""));

    FCodeForgeTemplateResult Result = Engine.Process(
        TEXT("{{ReturnType}} {{FunctionName}}({{Params}});"), Context);

    TestTrue(TEXT("bSuccess"), Result.bSuccess);
    TestEqual(TEXT("Output"), Result.Output,
        FString(TEXT("float GetHealth();")));
    return true;
}

// ============================================================
//  Test 3: Conditional â€” True
// ============================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTemplateTest_ConditionalTrue,
    "CodeForge.TemplateEngine.ConditionalTrue",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTemplateTest_ConditionalTrue::RunTest(const FString& Parameters)
{
    FCodeForgeTemplateEngine Engine;
    FCodeForgeTemplateContext Context;
    Context.Conditions.Add(TEXT("Replicated"), true);

    FCodeForgeTemplateResult Result = Engine.Process(
        TEXT("{{#if Replicated}}bReplicates = true;{{/if}}"), Context);

    TestTrue(TEXT("bSuccess"), Result.bSuccess);
    TestEqual(TEXT("Output"), Result.Output,
        FString(TEXT("bReplicates = true;")));
    return true;
}

// ============================================================
//  Test 4: Conditional â€” False
// ============================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTemplateTest_ConditionalFalse,
    "CodeForge.TemplateEngine.ConditionalFalse",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTemplateTest_ConditionalFalse::RunTest(const FString& Parameters)
{
    FCodeForgeTemplateEngine Engine;
    FCodeForgeTemplateContext Context;
    Context.Conditions.Add(TEXT("Replicated"), false);

    FCodeForgeTemplateResult Result = Engine.Process(
        TEXT("{{#if Replicated}}bReplicates = true;{{/if}}"), Context);

    TestTrue(TEXT("bSuccess"), Result.bSuccess);
    TestEqual(TEXT("Output"), Result.Output, FString(TEXT("")));
    return true;
}

// ============================================================
//  Test 5: Negated Conditional
// ============================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTemplateTest_NegatedConditional,
    "CodeForge.TemplateEngine.NegatedConditional",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTemplateTest_NegatedConditional::RunTest(const FString& Parameters)
{
    FCodeForgeTemplateEngine Engine;
    FCodeForgeTemplateContext Context;
    Context.Conditions.Add(TEXT("Replicated"), false);

    FCodeForgeTemplateResult Result = Engine.Process(
        TEXT("{{#if !Replicated}}// Not replicated{{/if}}"), Context);

    TestTrue(TEXT("bSuccess"), Result.bSuccess);
    TestEqual(TEXT("Output"), Result.Output,
        FString(TEXT("// Not replicated")));
    return true;
}

// ============================================================
//  Test 6: Loop
// ============================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTemplateTest_Loop,
    "CodeForge.TemplateEngine.Loop",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTemplateTest_Loop::RunTest(const FString& Parameters)
{
    FCodeForgeTemplateEngine Engine;
    FCodeForgeTemplateContext Context;

    FCodeForgeTemplateArrayItem Item0;
    Item0.Values.Add(TEXT("PropertyType"), TEXT("float"));
    Item0.Values.Add(TEXT("PropertyName"), TEXT("Health"));

    FCodeForgeTemplateArrayItem Item1;
    Item1.Values.Add(TEXT("PropertyType"), TEXT("int32"));
    Item1.Values.Add(TEXT("PropertyName"), TEXT("Score"));

    Context.Arrays.Add(TEXT("Properties"), {Item0, Item1});

    FCodeForgeTemplateResult Result = Engine.Process(
        TEXT("{{#each Properties}}    {{PropertyType}} {{PropertyName}};\n{{/each}}"), Context);

    TestTrue(TEXT("bSuccess"), Result.bSuccess);
    TestEqual(TEXT("Output"), Result.Output,
        FString(TEXT("    float Health;\n    int32 Score;\n")));
    return true;
}

// ============================================================
//  Test 7: Loop with Parent Variable Fallback
// ============================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTemplateTest_LoopParentFallback,
    "CodeForge.TemplateEngine.LoopParentFallback",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTemplateTest_LoopParentFallback::RunTest(const FString& Parameters)
{
    FCodeForgeTemplateEngine Engine;
    FCodeForgeTemplateContext Context;
    Context.Variables.Add(TEXT("OwnerClass"), TEXT("AMyActor"));

    FCodeForgeTemplateArrayItem Item0;
    Item0.Values.Add(TEXT("PropertyName"), TEXT("Health"));

    FCodeForgeTemplateArrayItem Item1;
    Item1.Values.Add(TEXT("PropertyName"), TEXT("Score"));

    Context.Arrays.Add(TEXT("Properties"), {Item0, Item1});

    FCodeForgeTemplateResult Result = Engine.Process(
        TEXT("{{#each Properties}}DOREPLIFETIME({{OwnerClass}}, {{PropertyName}});{{/each}}"),
        Context);

    TestTrue(TEXT("bSuccess"), Result.bSuccess);
    TestEqual(TEXT("Output"), Result.Output,
        FString(TEXT("DOREPLIFETIME(AMyActor, Health);DOREPLIFETIME(AMyActor, Score);")));
    return true;
}

// ============================================================
//  Test 8: Include
// ============================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTemplateTest_Include,
    "CodeForge.TemplateEngine.Include",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTemplateTest_Include::RunTest(const FString& Parameters)
{
    FCodeForgeTemplateEngine Engine;
    Engine.SetTemplateResolver([](const FString& Name) -> FString
    {
        if (Name == TEXT("copyright"))
        {
            return TEXT("// Copyright 2026");
        }
        return TEXT("");
    });

    FCodeForgeTemplateContext Context;

    FCodeForgeTemplateResult Result = Engine.Process(
        TEXT("{{> copyright}}\nclass Foo {};"), Context);

    TestTrue(TEXT("bSuccess"), Result.bSuccess);
    TestEqual(TEXT("Output"), Result.Output,
        FString(TEXT("// Copyright 2026\nclass Foo {};")));
    return true;
}

// ============================================================
//  Test 9: Conditional with Variables Inside
// ============================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTemplateTest_ConditionalWithVariables,
    "CodeForge.TemplateEngine.ConditionalWithVariables",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTemplateTest_ConditionalWithVariables::RunTest(const FString& Parameters)
{
    FCodeForgeTemplateEngine Engine;
    FCodeForgeTemplateContext Context;
    Context.Variables.Add(TEXT("ClassName"), TEXT("AMyActor"));
    Context.Conditions.Add(TEXT("Replicated"), true);

    FCodeForgeTemplateResult Result = Engine.Process(
        TEXT("{{#if Replicated}}void {{ClassName}}::GetLifetimeReplicatedProps() {}{{/if}}"),
        Context);

    TestTrue(TEXT("bSuccess"), Result.bSuccess);
    TestEqual(TEXT("Output"), Result.Output,
        FString(TEXT("void AMyActor::GetLifetimeReplicatedProps() {}")));
    return true;
}

// ============================================================
//  Test 10: Error â€” Unknown Variable
// ============================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTemplateTest_ErrorUnknownVariable,
    "CodeForge.TemplateEngine.ErrorUnknownVariable",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTemplateTest_ErrorUnknownVariable::RunTest(const FString& Parameters)
{
    FCodeForgeTemplateEngine Engine;
    FCodeForgeTemplateContext Context; // empty context

    FCodeForgeTemplateResult Result = Engine.Process(
        TEXT("class {{ClassName}} {};"), Context);

    TestFalse(TEXT("bSuccess is false"), Result.bSuccess);
    TestTrue(TEXT("ErrorMessage mentions ClassName"),
        Result.ErrorMessage.Contains(TEXT("ClassName")));
    return true;
}

// ============================================================
//  Test 11: Error â€” Unclosed If
// ============================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTemplateTest_ErrorUnclosedIf,
    "CodeForge.TemplateEngine.ErrorUnclosedIf",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTemplateTest_ErrorUnclosedIf::RunTest(const FString& Parameters)
{
    FCodeForgeTemplateEngine Engine;
    FCodeForgeTemplateContext Context;
    Context.Conditions.Add(TEXT("Foo"), true);

    FCodeForgeTemplateResult Result = Engine.Process(
        TEXT("{{#if Foo}}unclosed block"), Context);

    TestFalse(TEXT("bSuccess is false"), Result.bSuccess);
    // ErrorMessage should describe the unclosed block
    TestTrue(TEXT("ErrorMessage mentions unclosed/if"),
        Result.ErrorMessage.Contains(TEXT("if")) ||
        Result.ErrorMessage.Contains(TEXT("If")) ||
        Result.ErrorMessage.Contains(TEXT("unclosed")) ||
        Result.ErrorMessage.Contains(TEXT("Unclosed")));
    return true;
}

// ============================================================
//  Test 12: Error â€” Unclosed Each
// ============================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTemplateTest_ErrorUnclosedEach,
    "CodeForge.TemplateEngine.ErrorUnclosedEach",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTemplateTest_ErrorUnclosedEach::RunTest(const FString& Parameters)
{
    FCodeForgeTemplateEngine Engine;
    FCodeForgeTemplateContext Context;

    FCodeForgeTemplateResult Result = Engine.Process(
        TEXT("{{#each Items}}no end"), Context);

    TestFalse(TEXT("bSuccess is false"), Result.bSuccess);
    return true;
}

// ============================================================
//  Test 13: Error â€” Missing Include
// ============================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTemplateTest_ErrorMissingInclude,
    "CodeForge.TemplateEngine.ErrorMissingInclude",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTemplateTest_ErrorMissingInclude::RunTest(const FString& Parameters)
{
    FCodeForgeTemplateEngine Engine;
    // No resolver set
    FCodeForgeTemplateContext Context;

    FCodeForgeTemplateResult Result = Engine.Process(
        TEXT("{{> missing_template}}"), Context);

    TestFalse(TEXT("bSuccess is false"), Result.bSuccess);
    TestTrue(TEXT("ErrorMessage mentions template name"),
        Result.ErrorMessage.Contains(TEXT("missing_template")));
    return true;
}

// ============================================================
//  Test 14: Empty Template
// ============================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTemplateTest_EmptyTemplate,
    "CodeForge.TemplateEngine.EmptyTemplate",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTemplateTest_EmptyTemplate::RunTest(const FString& Parameters)
{
    FCodeForgeTemplateEngine Engine;
    FCodeForgeTemplateContext Context;

    FCodeForgeTemplateResult Result = Engine.Process(TEXT(""), Context);

    TestTrue(TEXT("bSuccess"), Result.bSuccess);
    TestEqual(TEXT("Output is empty"), Result.Output, FString(TEXT("")));
    return true;
}

// ============================================================
//  Test 15: No Placeholders
// ============================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTemplateTest_NoPlaceholders,
    "CodeForge.TemplateEngine.NoPlaceholders",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTemplateTest_NoPlaceholders::RunTest(const FString& Parameters)
{
    FCodeForgeTemplateEngine Engine;
    FCodeForgeTemplateContext Context;

    const FString Input = TEXT("#pragma once\n#include \"CoreMinimal.h\"");
    FCodeForgeTemplateResult Result = Engine.Process(Input, Context);

    TestTrue(TEXT("bSuccess"), Result.bSuccess);
    TestEqual(TEXT("Output equals input"), Result.Output, Input);
    return true;
}

// ============================================================
//  Test 16: Simple API (TMap<FString,FString> overload)
// ============================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTemplateTest_SimpleAPI,
    "CodeForge.TemplateEngine.SimpleAPI",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTemplateTest_SimpleAPI::RunTest(const FString& Parameters)
{
    FCodeForgeTemplateEngine Engine;

    TMap<FString, FString> Vars;
    Vars.Add(TEXT("Name"), TEXT("World"));

    FCodeForgeTemplateResult Result = Engine.Process(TEXT("Hello {{Name}}"), Vars);

    TestTrue(TEXT("bSuccess"), Result.bSuccess);
    TestEqual(TEXT("Output"), Result.Output, FString(TEXT("Hello World")));
    return true;
}

// ============================================================
//  Test 17: Empty Loop (zero items)
// ============================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTemplateTest_EmptyLoop,
    "CodeForge.TemplateEngine.EmptyLoop",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTemplateTest_EmptyLoop::RunTest(const FString& Parameters)
{
    FCodeForgeTemplateEngine Engine;
    FCodeForgeTemplateContext Context;

    // Add an empty array
    Context.Arrays.Add(TEXT("Items"), TArray<FCodeForgeTemplateArrayItem>());

    FCodeForgeTemplateResult Result = Engine.Process(
        TEXT("Before{{#each Items}}X{{/each}}After"), Context);

    TestTrue(TEXT("bSuccess"), Result.bSuccess);
    TestEqual(TEXT("Output"), Result.Output, FString(TEXT("BeforeAfter")));
    return true;
}

// ============================================================
//  Test 18: Nested Variable in Loop Not Found
// ============================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTemplateTest_LoopUnknownVar,
    "CodeForge.TemplateEngine.LoopUnknownVar",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTemplateTest_LoopUnknownVar::RunTest(const FString& Parameters)
{
    FCodeForgeTemplateEngine Engine;
    FCodeForgeTemplateContext Context;

    FCodeForgeTemplateArrayItem Item;
    Item.Values.Add(TEXT("Name"), TEXT("A"));
    Context.Arrays.Add(TEXT("Items"), {Item});

    FCodeForgeTemplateResult Result = Engine.Process(
        TEXT("{{#each Items}}{{UnknownVar}}{{/each}}"), Context);

    TestFalse(TEXT("bSuccess is false"),  Result.bSuccess);
    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS



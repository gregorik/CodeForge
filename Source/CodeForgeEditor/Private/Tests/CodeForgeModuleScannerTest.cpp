// Copyright (c) 2026 GregOrigin. All Rights Reserved.

// CodeForgeModuleScannerTest.cpp
// Automation tests for FCodeForgeModuleScanner â€” Task 7 (TDD).
//
// Tests exercise the module scanning pipeline:
//   ProjectDir â†’ FCodeForgeModuleScanner â†’ TArray<FCodeForgeModuleInfo>
//
// Running inside the UE5 editor, FPaths::ProjectDir() points to the
// CodeForge project which has a "CodeForge" game module, so at least one
// module must be found.

#include "Misc/AutomationTest.h"
#include "CodeForgeModuleScanner.h"
#include "Misc/Paths.h"

#if WITH_DEV_AUTOMATION_TESTS

// ===========================================================================
// Test 1 â€” ScanProject finds at least one module
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeModuleScannerTest_FindsModules,
    "CodeForge.ModuleScanner.FindsModules",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeModuleScannerTest_FindsModules::RunTest(const FString& Parameters)
{
    FCodeForgeModuleScanner Scanner;
    FString ProjectDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
    FPaths::NormalizeFilename(ProjectDir);

    TArray<FCodeForgeModuleInfo> Modules = Scanner.ScanProject(ProjectDir);

    TestTrue(TEXT("At least one module found in project"), Modules.Num() > 0);
    return true;
}

// ===========================================================================
// Test 2 â€” Each discovered module has a non-empty ModuleName
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeModuleScannerTest_ModuleNamesNonEmpty,
    "CodeForge.ModuleScanner.ModuleNamesNonEmpty",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeModuleScannerTest_ModuleNamesNonEmpty::RunTest(const FString& Parameters)
{
    FCodeForgeModuleScanner Scanner;
    FString ProjectDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
    FPaths::NormalizeFilename(ProjectDir);

    TArray<FCodeForgeModuleInfo> Modules = Scanner.ScanProject(ProjectDir);

    for (const FCodeForgeModuleInfo& Info : Modules)
    {
        TestFalse(
            FString::Printf(TEXT("ModuleName must not be empty (got empty for entry)")),
            Info.ModuleName.IsEmpty());
    }
    return true;
}

// ===========================================================================
// Test 3 â€” Each discovered module has a non-empty PublicDir
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeModuleScannerTest_PublicDirNonEmpty,
    "CodeForge.ModuleScanner.PublicDirNonEmpty",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeModuleScannerTest_PublicDirNonEmpty::RunTest(const FString& Parameters)
{
    FCodeForgeModuleScanner Scanner;
    FString ProjectDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
    FPaths::NormalizeFilename(ProjectDir);

    TArray<FCodeForgeModuleInfo> Modules = Scanner.ScanProject(ProjectDir);

    for (const FCodeForgeModuleInfo& Info : Modules)
    {
        TestFalse(
            FString::Printf(TEXT("PublicDir must not be empty for module '%s'"), *Info.ModuleName),
            Info.PublicDir.IsEmpty());
    }
    return true;
}

// ===========================================================================
// Test 4 â€” Each discovered module has a non-empty APIMacro
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeModuleScannerTest_APIMacroNonEmpty,
    "CodeForge.ModuleScanner.APIMacroNonEmpty",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeModuleScannerTest_APIMacroNonEmpty::RunTest(const FString& Parameters)
{
    FCodeForgeModuleScanner Scanner;
    FString ProjectDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
    FPaths::NormalizeFilename(ProjectDir);

    TArray<FCodeForgeModuleInfo> Modules = Scanner.ScanProject(ProjectDir);

    for (const FCodeForgeModuleInfo& Info : Modules)
    {
        TestFalse(
            FString::Printf(TEXT("APIMacro must not be empty for module '%s'"), *Info.ModuleName),
            Info.APIMacro.IsEmpty());
    }
    return true;
}

// ===========================================================================
// Test 5 â€” APIMacro follows UPPERCASE_API format
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeModuleScannerTest_APIMacroFormat,
    "CodeForge.ModuleScanner.APIMacroFormat",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeModuleScannerTest_APIMacroFormat::RunTest(const FString& Parameters)
{
    FCodeForgeModuleScanner Scanner;
    FString ProjectDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
    FPaths::NormalizeFilename(ProjectDir);

    TArray<FCodeForgeModuleInfo> Modules = Scanner.ScanProject(ProjectDir);

    for (const FCodeForgeModuleInfo& Info : Modules)
    {
        // APIMacro must end with "_API"
        TestTrue(
            FString::Printf(TEXT("APIMacro for '%s' must end with _API, got: '%s'"),
                *Info.ModuleName, *Info.APIMacro),
            Info.APIMacro.EndsWith(TEXT("_API")));

        // APIMacro must be all uppercase (plus underscore)
        FString Expected = Info.ModuleName.ToUpper() + TEXT("_API");
        TestEqual(
            FString::Printf(TEXT("APIMacro for '%s' must be MODULENAME_API"), *Info.ModuleName),
            Info.APIMacro,
            Expected);
    }
    return true;
}

// ===========================================================================
// Test 6 â€” DeriveAPIMacro produces correct output for known inputs
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeModuleScannerTest_DeriveAPIMacro,
    "CodeForge.ModuleScanner.DeriveAPIMacro",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeModuleScannerTest_DeriveAPIMacro::RunTest(const FString& Parameters)
{
    // We exercise DeriveAPIMacro indirectly by scanning the known project
    // and checking the CodeForge module entry specifically.
    FCodeForgeModuleScanner Scanner;
    FString ProjectDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
    FPaths::NormalizeFilename(ProjectDir);

    TArray<FCodeForgeModuleInfo> Modules = Scanner.ScanProject(ProjectDir);

    // The CodeForge.uproject defines a module named "CodeForge"
    const FCodeForgeModuleInfo* CodeForgeModule = Modules.FindByPredicate(
        [](const FCodeForgeModuleInfo& M) { return M.ModuleName == TEXT("CodeForge"); });

    TestNotNull(TEXT("Scanner must find the 'CodeForge' module"), CodeForgeModule);

    if (CodeForgeModule)
    {
        TestEqual(TEXT("APIMacro for CodeForge module must be CODEFORGE_API"),
            CodeForgeModule->APIMacro,
            FString(TEXT("CODEFORGE_API")));
    }

    return true;
}

// ===========================================================================
// Test 7 â€” SourceDir, PublicDir, and PrivateDir paths are consistent
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeModuleScannerTest_DirConsistency,
    "CodeForge.ModuleScanner.DirConsistency",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeModuleScannerTest_DirConsistency::RunTest(const FString& Parameters)
{
    FCodeForgeModuleScanner Scanner;
    FString ProjectDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
    FPaths::NormalizeFilename(ProjectDir);

    TArray<FCodeForgeModuleInfo> Modules = Scanner.ScanProject(ProjectDir);

    for (const FCodeForgeModuleInfo& Info : Modules)
    {
        FString CandidatePublic  = FPaths::Combine(Info.SourceDir, TEXT("Public"));
        FString CandidatePrivate = FPaths::Combine(Info.SourceDir, TEXT("Private"));

        // PublicDir must be SourceDir/Public if that directory exists, else SourceDir (fallback)
        FString ExpectedPublic = FPaths::DirectoryExists(CandidatePublic) ? CandidatePublic : Info.SourceDir;
        TestEqual(
            FString::Printf(TEXT("PublicDir for '%s' must match expected path"), *Info.ModuleName),
            Info.PublicDir,
            ExpectedPublic);

        // PrivateDir must be SourceDir/Private if that directory exists, else SourceDir (fallback)
        FString ExpectedPrivate = FPaths::DirectoryExists(CandidatePrivate) ? CandidatePrivate : Info.SourceDir;
        TestEqual(
            FString::Printf(TEXT("PrivateDir for '%s' must match expected path"), *Info.ModuleName),
            Info.PrivateDir,
            ExpectedPrivate);

        // SourceDir must contain the ModuleName
        TestTrue(
            FString::Printf(TEXT("SourceDir for '%s' must contain module name"), *Info.ModuleName),
            Info.SourceDir.Contains(Info.ModuleName));
    }
    return true;
}

// ===========================================================================
// Test 8 â€” Scanning a non-existent directory returns empty result gracefully
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeModuleScannerTest_EmptyOnMissingDir,
    "CodeForge.ModuleScanner.EmptyOnMissingDir",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeModuleScannerTest_EmptyOnMissingDir::RunTest(const FString& Parameters)
{
    FCodeForgeModuleScanner Scanner;

    TArray<FCodeForgeModuleInfo> Modules = Scanner.ScanProject(
        TEXT("Z:/DoesNotExist/FakeProject_12345"));

    TestEqual(TEXT("Scanning missing directory returns empty array"), Modules.Num(), 0);
    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS



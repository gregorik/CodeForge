// Copyright (c) 2026 GregOrigin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CodeForgeTypes.h"
#include "CodeForgeTemplateEngine.h"
#include "CodeForgeChangeDetector.h"

// Forward declarations
class UCodeForgeBlueprint;

/**
 * Result of a code generation pass.
 * On success, HeaderContent and SourceContent contain the generated file text.
 * SourceContent may be empty for Enum kinds (no .cpp needed).
 */
struct FCodeForgeGeneratorResult
{
    bool    bSuccess        = false;
    FString HeaderContent;
    FString SourceContent;
    FString HeaderPath;
    FString SourcePath;
    FString ErrorMessage;

    /** True when the generation changed the class interface (structural change). */
    bool bIsStructuralChange = false;
};

/**
 * FCodeForgeGenerator
 *
 * Layer 2â†’3â†’4 bridge: reads a UCodeForgeBlueprint schema, builds a
 * FCodeForgeTemplateContext, runs it through FCodeForgeTemplateEngine using
 * the .cft template library, and returns ready-to-write .h/.cpp text.
 *
 * This class lives in the Editor module and is never shipped with a game.
 */
class CODEFORGEEDITOR_API FCodeForgeGenerator
{
public:

    /**
     * Set the root directory that contains the template sub-folders
     * (Common/, Class/, Struct/, Enum/, Replication/).
     * Defaults to the plugin's Content/Templates directory when empty.
     */
    void SetTemplatePath(const FString& Path);

    /**
     * Generate header + source content from the Blueprint schema.
     * @param Blueprint  Must not be nullptr; must pass Validate().
     * @return           FCodeForgeGeneratorResult with bSuccess and file texts.
     */
    FCodeForgeGeneratorResult Generate(const UCodeForgeBlueprint* Blueprint) const;

    /**
     * Generate, write to project, and run change-detection in one call.
     *
     * @param Blueprint   Must not be nullptr; must pass Validate().
     * @param ProjectDir  Absolute path to the project root (containing .uproject).
     * @return            FCodeForgeGeneratorResult with bSuccess, file texts,
     *                    and bIsStructuralChange set.
     */
    FCodeForgeGeneratorResult GenerateAndIntegrate(const UCodeForgeBlueprint* Blueprint, const FString& ProjectDir);

    /**
     * Write a previously generated result to disk.
     *
     * @param Result      Successful result from Generate().
     * @param PublicDir   Root public include directory (e.g. Source/MyGame/Public).
     * @param PrivateDir  Root private source directory (e.g. Source/MyGame/Private).
     * @param SubDir      Optional sub-directory appended to both paths.
     * @return            True on success.
     */
    bool WriteToProject(const FCodeForgeGeneratorResult& Result,
        const FString& PublicDir, const FString& PrivateDir,
        const FString& SubDir = FString()) const;

private:

    /** Root path that contains Common/, Class/, Struct/, Enum/, Replication/ sub-folders. */
    FString TemplatePath;

    /** Change detector that tracks structural signatures across generations. */
    FCodeForgeChangeDetector ChangeDetector;

    // -----------------------------------------------------------------------
    // Internal helpers
    // -----------------------------------------------------------------------

    /** Build the full template context from the Blueprint schema. */
    FCodeForgeTemplateContext BuildContext(const UCodeForgeBlueprint* Blueprint) const;

    /**
     * Return the template file name (relative to TemplatePath) for the
     * given Blueprint kind.  bHeader selects .h vs .cpp template.
     * Returns an empty string when no source template is needed (e.g. Enum).
     */
    FString SelectTemplate(const UCodeForgeBlueprint* Blueprint, bool bHeader) const;

    /**
     * Load a template file by name (without extension / path), searching
     * Common/, Class/, Struct/, Enum/, Replication/ sub-directories.
     * Returns the file content, or an empty string if not found.
     */
    FString LoadTemplate(const FString& TemplateName) const;

    /**
     * Map an ECodeForgeRepCondition enum value to its COND_* string
     * for use in DOREPLIFETIME_CONDITION() macro calls.
     */
    static FString RepConditionToString(ECodeForgeRepCondition Condition);
};




// Copyright (c) 2026 GregOrigin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CodeForgeTypes.h"
#include "CodeForgePropertyDef.h"
#include "CodeForgeFunctionDef.h"
#include "CodeForgeEnumDef.h"
#include "CodeForgeBlueprint.generated.h"

/**
 * A single validation message produced by UCodeForgeBlueprint::Validate().
 * AutoFixId is non-empty when an automatic fix is available via ApplyAutoFix().
 */
USTRUCT()
struct FCodeForgeValidationResult
{
    GENERATED_BODY()

    UPROPERTY() ECodeForgeValidationSeverity Severity = ECodeForgeValidationSeverity::Error;
    UPROPERTY() FString Message;
    UPROPERTY() FString FieldName;
    UPROPERTY() FString AutoFixId;
};

/**
 * Root data asset that stores a user's class/struct/enum definition.
 * This is a pure data object â€” no UI, no code generation.
 * It is stored in the Runtime module so it can be loaded in any context.
 */
UCLASS(BlueprintType)
class CODEFORGERUNTIME_API UCodeForgeBlueprint : public UObject
{
    GENERATED_BODY()

public:
    // -------------------------------------------------------------------------
    // Identity
    // -------------------------------------------------------------------------

    UPROPERTY(EditAnywhere, Category = "CodeForge")
    FString ClassName;

    UPROPERTY(EditAnywhere, Category = "CodeForge")
    ECodeForgeBlueprintKind BlueprintKind = ECodeForgeBlueprintKind::Class;

    UPROPERTY(EditAnywhere, Category = "CodeForge|Class")
    ECodeForgeClassType ClassType = ECodeForgeClassType::Actor;

    /** Whether to add Replicated to UCLASS specifiers (adds AActor replication setup). */
    UPROPERTY(EditAnywhere, Category = "CodeForge|Class")
    bool bReplicated = false;

    /** Target module name for code generation output. */
    UPROPERTY(EditAnywhere, Category = "CodeForge")
    FString ModuleTarget;

    /** Optional sub-directory under the module's Public/ folder. */
    UPROPERTY(EditAnywhere, Category = "CodeForge")
    FString SubDirectory;

    /** Custom code injected into the constructor after bReplicates = true (if applicable). */
    UPROPERTY(EditAnywhere, Category = "CodeForge|Class", meta = (MultiLine = true))
    FString ConstructorBody;

    // -------------------------------------------------------------------------
    // Class members
    // -------------------------------------------------------------------------

    UPROPERTY(EditAnywhere, Category = "CodeForge|Class")
    TArray<FCodeForgePropertyDef> Properties;

    UPROPERTY(EditAnywhere, Category = "CodeForge|Class")
    TArray<FCodeForgeFunctionDef> Functions;

    // -------------------------------------------------------------------------
    // Struct members
    // -------------------------------------------------------------------------

    UPROPERTY(EditAnywhere, Category = "CodeForge|Struct")
    TArray<FCodeForgePropertyDef> StructProperties;

    // -------------------------------------------------------------------------
    // Enum members
    // -------------------------------------------------------------------------

    UPROPERTY(EditAnywhere, Category = "CodeForge|Enum")
    TArray<FCodeForgeEnumEntryDef> EnumEntries;

    UPROPERTY(EditAnywhere, Category = "CodeForge|Enum")
    bool bBlueprintType = true;

    // -------------------------------------------------------------------------
    // Helpers used by the code generator (Task 6)
    // -------------------------------------------------------------------------

    /** Returns the C++ parent class name, e.g. "AActor" for ECodeForgeClassType::Actor. */
    FString GetParentClassName() const;

    /** Returns the #include path for the parent class, e.g. "GameFramework/Actor.h". */
    FString GetParentIncludePath() const;

    /** Returns the UCLASS specifier string, always starts with "Blueprintable". */
    FString GetClassSpecifiers() const;

    // -------------------------------------------------------------------------
    // Validation (implemented in Task 4)
    // -------------------------------------------------------------------------

    TArray<FCodeForgeValidationResult> Validate() const;
    void ApplyAutoFix(const FString& AutoFixId);
};



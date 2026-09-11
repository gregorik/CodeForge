// Copyright (c) 2026 GregOrigin. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CodeForgeTypes.h"
#include "CodeForgePropertyDef.generated.h"

USTRUCT(BlueprintType)
struct CODEFORGERUNTIME_API FCodeForgeParamDef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") FString Name;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") FString Type;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bIsConst = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bIsRef = false;

    /** Builds the full parameter declaration string, e.g. "const FString& ParamName" */
    FString BuildParamString() const;
};

USTRUCT(BlueprintType)
struct CODEFORGERUNTIME_API FCodeForgePropertyDef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") FString Name;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") FString Type;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") FString Category;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") FString DefaultValue;

    // Edit specifiers (mutually exclusive — first true wins)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bVisibleAnywhere = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bEditAnywhere = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bEditDefaultsOnly = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bEditInstanceOnly = false;

    // Blueprint specifiers (mutually exclusive — first true wins)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bBlueprintReadWrite = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bBlueprintReadOnly = false;

    // Replication
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bReplicated = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") ECodeForgeRepCondition ReplicationCondition = ECodeForgeRepCondition::None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bRepNotify = false;

    // Other specifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bExposeOnSpawn = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bSaveGame = false;

    // Meta tags
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") TMap<FString, FString> Meta;

    /** Optional #include path for custom types (e.g. "MyStruct.h"). Empty for built-in types. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") FString IncludePath;

    /** Optional body emitted inside the generated OnRep_ function for RepNotify properties. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge", meta = (MultiLine = "true")) FString OnRepBody;

    /**
     * Builds the UPROPERTY specifier string, e.g.
     * "EditAnywhere, BlueprintReadWrite, Replicated, Category=\"Foo\", meta=(ClampMin=0)"
     */
    FString BuildSpecifierString() const;

    /**
     * Returns " = <DefaultValue>" if DefaultValue is non-empty, otherwise "".
     */
    FString BuildDefaultValueSuffix() const;
};

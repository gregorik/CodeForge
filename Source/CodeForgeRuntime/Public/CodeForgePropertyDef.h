#pragma once

#include "CoreMinimal.h"
#include "CodeForgeTypes.h"
#include "CodeForgePropertyDef.generated.h"

USTRUCT(BlueprintType)
struct CODEFORGERUNTIME_API FCodeForgeParamDef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere) FString Name;
    UPROPERTY(EditAnywhere) FString Type;
    UPROPERTY(EditAnywhere) bool bIsConst = false;
    UPROPERTY(EditAnywhere) bool bIsRef = false;

    /** Builds the full parameter declaration string, e.g. "const FString& ParamName" */
    FString BuildParamString() const;
};

USTRUCT(BlueprintType)
struct CODEFORGERUNTIME_API FCodeForgePropertyDef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere) FString Name;
    UPROPERTY(EditAnywhere) FString Type;
    UPROPERTY(EditAnywhere) FString Category;
    UPROPERTY(EditAnywhere) FString DefaultValue;

    // Edit specifiers (mutually exclusive — first true wins)
    UPROPERTY(EditAnywhere) bool bEditAnywhere = false;
    UPROPERTY(EditAnywhere) bool bEditDefaultsOnly = false;
    UPROPERTY(EditAnywhere) bool bEditInstanceOnly = false;

    // Blueprint specifiers (mutually exclusive — first true wins)
    UPROPERTY(EditAnywhere) bool bBlueprintReadWrite = false;
    UPROPERTY(EditAnywhere) bool bBlueprintReadOnly = false;

    // Replication
    UPROPERTY(EditAnywhere) bool bReplicated = false;
    UPROPERTY(EditAnywhere) ECodeForgeRepCondition ReplicationCondition = ECodeForgeRepCondition::None;
    UPROPERTY(EditAnywhere) bool bRepNotify = false;

    // Visibility specifier (mutually exclusive with Edit specifiers)
    UPROPERTY(EditAnywhere) bool bVisibleAnywhere = false;

    // Other specifiers
    UPROPERTY(EditAnywhere) bool bExposeOnSpawn = false;
    UPROPERTY(EditAnywhere) bool bSaveGame = false;

    /** Optional OnRep_ function body. Empty → "// TODO". Non-empty → verbatim. */
    UPROPERTY(EditAnywhere, meta = (MultiLine = true))
    FString OnRepBody;

    // Meta tags
    UPROPERTY(EditAnywhere) TMap<FString, FString> Meta;

    /** Optional #include path for custom types (e.g. "MyStruct.h"). Empty for built-in types. */
    UPROPERTY(EditAnywhere) FString IncludePath;

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


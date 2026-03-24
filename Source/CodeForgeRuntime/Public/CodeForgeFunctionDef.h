#pragma once

#include "CoreMinimal.h"
#include "CodeForgePropertyDef.h"
#include "CodeForgeFunctionDef.generated.h"

USTRUCT(BlueprintType)
struct CODEFORGERUNTIME_API FCodeForgeFunctionDef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere) FString Name;
    UPROPERTY(EditAnywhere) FString ReturnType = TEXT("void");
    UPROPERTY(EditAnywhere) TArray<FCodeForgeParamDef> Parameters;
    UPROPERTY(EditAnywhere) FString Category;

    UPROPERTY(EditAnywhere) bool bBlueprintCallable = true;
    UPROPERTY(EditAnywhere) bool bBlueprintPure = false;
    UPROPERTY(EditAnywhere) bool bConst = false;
    UPROPERTY(EditAnywhere) bool bBlueprintNativeEvent = false;
    UPROPERTY(EditAnywhere) bool bExec = false;

    /** Optional function body. Empty → generator emits "// TODO". Non-empty → emitted verbatim. */
    UPROPERTY(EditAnywhere, meta = (MultiLine = true))
    FString FunctionBody;

    /** Optional #include path for custom return types (e.g. "MyStruct.h"). Empty for built-in types. */
    UPROPERTY(EditAnywhere) FString IncludePath;

    /**
     * Builds the UFUNCTION specifier string, e.g.
     * "BlueprintCallable, BlueprintNativeEvent, Category=\"Foo\""
     */
    FString BuildSpecifierString() const;

    /**
     * Joins all Parameters' BuildParamString() with ", ".
     */
    FString BuildParamListString() const;
};


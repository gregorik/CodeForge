// Copyright (c) 2026 GregOrigin. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CodeForgePropertyDef.h"
#include "CodeForgeFunctionDef.generated.h"

USTRUCT(BlueprintType)
struct CODEFORGERUNTIME_API FCodeForgeFunctionDef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") FString Name;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") FString ReturnType = TEXT("void");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") TArray<FCodeForgeParamDef> Parameters;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") FString Category;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") FString IncludePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bBlueprintCallable = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bBlueprintPure = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bConst = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bBlueprintNativeEvent = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bExec = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bUFunction = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bVirtual = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bOverride = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge", meta = (MultiLine = "true")) FString FunctionBody;

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

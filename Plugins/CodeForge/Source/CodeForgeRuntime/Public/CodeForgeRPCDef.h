// Copyright (c) 2026 GregOrigin. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CodeForgeTypes.h"
#include "CodeForgePropertyDef.h"
#include "CodeForgeRPCDef.generated.h"

USTRUCT(BlueprintType)
struct CODEFORGERUNTIME_API FCodeForgeRPCDef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") FString Name;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") TArray<FCodeForgeParamDef> Parameters;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") ECodeForgeRPCMode Mode = ECodeForgeRPCMode::Server;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bReliable = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge", meta = (MultiLine = "true")) FString FunctionBody;

    /**
     * Builds the UFUNCTION specifier string for an RPC, e.g.
     * "Server, Reliable, WithValidation"
     * Server RPCs always include WithValidation per UE5 convention.
     */
    FString BuildSpecifierString() const;

    /**
     * Joins all Parameters' BuildParamString() with ", ".
     */
    FString BuildParamListString() const;
};

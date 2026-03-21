#pragma once

#include "CoreMinimal.h"
#include "CodeForgeTypes.h"
#include "CodeForgePropertyDef.h"
#include "CodeForgeRPCDef.generated.h"

USTRUCT(BlueprintType)
struct CODEFORGERUNTIME_API FCodeForgeRPCDef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere) FString Name;
    UPROPERTY(EditAnywhere) TArray<FCodeForgeParamDef> Parameters;
    UPROPERTY(EditAnywhere) ECodeForgeRPCMode Mode = ECodeForgeRPCMode::Server;
    UPROPERTY(EditAnywhere) bool bReliable = true;

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

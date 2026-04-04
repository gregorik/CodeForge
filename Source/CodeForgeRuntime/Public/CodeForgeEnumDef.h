// Copyright (c) 2026 GregOrigin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CodeForgeEnumDef.generated.h"

USTRUCT(BlueprintType)
struct CODEFORGERUNTIME_API FCodeForgeEnumEntryDef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere) FString Name;
    UPROPERTY(EditAnywhere) FString DisplayName;
    UPROPERTY(EditAnywhere) int64 ExplicitValue = 0;
    UPROPERTY(EditAnywhere) bool bHasExplicitValue = false;
    UPROPERTY(EditAnywhere) bool bHidden = false;
};



// Copyright (c) 2026 GregOrigin. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CodeForgeEnumDef.generated.h"

USTRUCT(BlueprintType)
struct CODEFORGERUNTIME_API FCodeForgeEnumEntryDef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") FString Name;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") FString DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") int64 ExplicitValue = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bHasExplicitValue = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CodeForge") bool bHidden = false;
};

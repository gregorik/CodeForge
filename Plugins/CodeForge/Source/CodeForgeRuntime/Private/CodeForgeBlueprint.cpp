#include "CodeForgeBlueprint.h"

// ---------------------------------------------------------------------------
// FCodeForgeParamDef
// ---------------------------------------------------------------------------

FString FCodeForgeParamDef::BuildParamString() const
{
    // Produce:
    //   const Type& Name   — const + ref
    //   const Type  Name   — const only
    //   Type&       Name   — ref only
    //   Type        Name   — neither

    FString Result;

    if (bIsConst)
    {
        Result += TEXT("const ");
    }

    Result += Type;

    if (bIsRef)
    {
        Result += TEXT("&");
    }

    Result += TEXT(" ");
    Result += Name;

    return Result;
}

// ---------------------------------------------------------------------------
// FCodeForgePropertyDef
// ---------------------------------------------------------------------------

FString FCodeForgePropertyDef::BuildSpecifierString() const
{
    TArray<FString> Specifiers;

    // Edit specifiers — mutually exclusive, first true wins
    if (bEditAnywhere)
    {
        Specifiers.Add(TEXT("EditAnywhere"));
    }
    else if (bEditDefaultsOnly)
    {
        Specifiers.Add(TEXT("EditDefaultsOnly"));
    }
    else if (bEditInstanceOnly)
    {
        Specifiers.Add(TEXT("EditInstanceOnly"));
    }

    // Blueprint specifiers — mutually exclusive, first true wins
    if (bBlueprintReadWrite)
    {
        Specifiers.Add(TEXT("BlueprintReadWrite"));
    }
    else if (bBlueprintReadOnly)
    {
        Specifiers.Add(TEXT("BlueprintReadOnly"));
    }

    // Replication — RepNotify takes priority over plain Replicated
    if (bReplicated)
    {
        if (bRepNotify)
        {
            Specifiers.Add(FString::Printf(TEXT("ReplicatedUsing=OnRep_%s"), *Name));
        }
        else
        {
            Specifiers.Add(TEXT("Replicated"));
        }
    }

    if (bExposeOnSpawn)
    {
        Specifiers.Add(TEXT("ExposeOnSpawn"));
    }

    if (bSaveGame)
    {
        Specifiers.Add(TEXT("SaveGame"));
    }

    if (!Category.IsEmpty())
    {
        Specifiers.Add(FString::Printf(TEXT("Category=\"%s\""), *Category));
    }

    // Meta tags
    if (Meta.Num() > 0)
    {
        TArray<FString> MetaPairs;
        for (const TPair<FString, FString>& Pair : Meta)
        {
            if (Pair.Value.IsEmpty())
            {
                MetaPairs.Add(Pair.Key);
            }
            else
            {
                MetaPairs.Add(FString::Printf(TEXT("%s=%s"), *Pair.Key, *Pair.Value));
            }
        }
        Specifiers.Add(FString::Printf(TEXT("meta=(%s)"), *FString::Join(MetaPairs, TEXT(", "))));
    }

    return FString::Join(Specifiers, TEXT(", "));
}

FString FCodeForgePropertyDef::BuildDefaultValueSuffix() const
{
    if (DefaultValue.IsEmpty())
    {
        return FString();
    }
    return FString::Printf(TEXT(" = %s"), *DefaultValue);
}

// ---------------------------------------------------------------------------
// FCodeForgeFunctionDef
// ---------------------------------------------------------------------------

FString FCodeForgeFunctionDef::BuildSpecifierString() const
{
    TArray<FString> Specifiers;

    // BlueprintPure implies BlueprintCallable — treat as mutually exclusive
    if (bBlueprintPure)
    {
        Specifiers.Add(TEXT("BlueprintPure"));
    }
    else if (bBlueprintCallable)
    {
        Specifiers.Add(TEXT("BlueprintCallable"));
    }

    if (bBlueprintNativeEvent)
    {
        Specifiers.Add(TEXT("BlueprintNativeEvent"));
    }

    if (!Category.IsEmpty())
    {
        Specifiers.Add(FString::Printf(TEXT("Category=\"%s\""), *Category));
    }

    return FString::Join(Specifiers, TEXT(", "));
}

FString FCodeForgeFunctionDef::BuildParamListString() const
{
    TArray<FString> Parts;
    Parts.Reserve(Parameters.Num());
    for (const FCodeForgeParamDef& Param : Parameters)
    {
        Parts.Add(Param.BuildParamString());
    }
    return FString::Join(Parts, TEXT(", "));
}

// ---------------------------------------------------------------------------
// FCodeForgeRPCDef
// ---------------------------------------------------------------------------

FString FCodeForgeRPCDef::BuildSpecifierString() const
{
    TArray<FString> Specifiers;

    switch (Mode)
    {
    case ECodeForgeRPCMode::Server:
        Specifiers.Add(TEXT("Server"));
        break;
    case ECodeForgeRPCMode::Client:
        Specifiers.Add(TEXT("Client"));
        break;
    case ECodeForgeRPCMode::NetMulticast:
        Specifiers.Add(TEXT("NetMulticast"));
        break;
    }

    Specifiers.Add(bReliable ? TEXT("Reliable") : TEXT("Unreliable"));

    // UE5 convention: Server RPCs always require WithValidation
    if (Mode == ECodeForgeRPCMode::Server)
    {
        Specifiers.Add(TEXT("WithValidation"));
    }

    return FString::Join(Specifiers, TEXT(", "));
}

FString FCodeForgeRPCDef::BuildParamListString() const
{
    TArray<FString> Parts;
    Parts.Reserve(Parameters.Num());
    for (const FCodeForgeParamDef& Param : Parameters)
    {
        Parts.Add(Param.BuildParamString());
    }
    return FString::Join(Parts, TEXT(", "));
}

// ---------------------------------------------------------------------------
// UCodeForgeBlueprint — helpers
// ---------------------------------------------------------------------------

FString UCodeForgeBlueprint::GetParentClassName() const
{
    switch (ClassType)
    {
    case ECodeForgeClassType::Actor:            return TEXT("AActor");
    case ECodeForgeClassType::Pawn:             return TEXT("APawn");
    case ECodeForgeClassType::Character:        return TEXT("ACharacter");
    case ECodeForgeClassType::ActorComponent:   return TEXT("UActorComponent");
    case ECodeForgeClassType::SceneComponent:   return TEXT("USceneComponent");
    case ECodeForgeClassType::Object:           return TEXT("UObject");
    case ECodeForgeClassType::GameModeBase:     return TEXT("AGameModeBase");
    case ECodeForgeClassType::GameStateBase:    return TEXT("AGameStateBase");
    case ECodeForgeClassType::PlayerController: return TEXT("APlayerController");
    case ECodeForgeClassType::PlayerState:      return TEXT("APlayerState");
    case ECodeForgeClassType::HUD:              return TEXT("AHUD");
    default:                                    return TEXT("UObject");
    }
}

FString UCodeForgeBlueprint::GetParentIncludePath() const
{
    switch (ClassType)
    {
    case ECodeForgeClassType::Actor:            return TEXT("GameFramework/Actor.h");
    case ECodeForgeClassType::Pawn:             return TEXT("GameFramework/Pawn.h");
    case ECodeForgeClassType::Character:        return TEXT("GameFramework/Character.h");
    case ECodeForgeClassType::ActorComponent:   return TEXT("Components/ActorComponent.h");
    case ECodeForgeClassType::SceneComponent:   return TEXT("Components/SceneComponent.h");
    case ECodeForgeClassType::Object:           return TEXT("UObject/NoExportTypes.h");
    case ECodeForgeClassType::GameModeBase:     return TEXT("GameFramework/GameModeBase.h");
    case ECodeForgeClassType::GameStateBase:    return TEXT("GameFramework/GameStateBase.h");
    case ECodeForgeClassType::PlayerController: return TEXT("GameFramework/PlayerController.h");
    case ECodeForgeClassType::PlayerState:      return TEXT("GameFramework/PlayerState.h");
    case ECodeForgeClassType::HUD:              return TEXT("GameFramework/HUD.h");
    default:                                    return TEXT("UObject/NoExportTypes.h");
    }
}

FString UCodeForgeBlueprint::GetClassSpecifiers() const
{
    TArray<FString> Specifiers;
    Specifiers.Add(TEXT("Blueprintable"));

    if (bReplicated)
    {
        Specifiers.Add(TEXT("Replicated"));
    }

    return FString::Join(Specifiers, TEXT(", "));
}

// ---------------------------------------------------------------------------
// UCodeForgeBlueprint — validation stubs (implemented in Task 4)
// ---------------------------------------------------------------------------

TArray<FCodeForgeValidationResult> UCodeForgeBlueprint::Validate() const
{
    TArray<FCodeForgeValidationResult> Results;
    // TODO: Implemented in Task 4
    return Results;
}

void UCodeForgeBlueprint::ApplyAutoFix(const FString& AutoFixId)
{
    // TODO: Implemented in Task 4
}

// Copyright (c) 2026 GregOrigin. All Rights Reserved.
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

    // Edit/visibility specifiers — mutually exclusive, first true wins
    if (bVisibleAnywhere)
    {
        Specifiers.Add(TEXT("VisibleAnywhere"));
    }
    else if (bEditAnywhere)
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

    // Exec and Blueprint exposure specifiers are mutually exclusive in CodeForge's UI.
    if (bExec)
    {
        Specifiers.Add(TEXT("Exec"));
    }
    else if (bBlueprintPure)
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
// UCodeForgeBlueprint — validation helpers
// ---------------------------------------------------------------------------

namespace CodeForgeValidation
{
    /** Returns true if this class type is Actor-derived (allowed to have RPCs). */
    static bool IsActorDerived(ECodeForgeClassType ClassType)
    {
        switch (ClassType)
        {
        case ECodeForgeClassType::Actor:
        case ECodeForgeClassType::Pawn:
        case ECodeForgeClassType::Character:
        case ECodeForgeClassType::GameModeBase:
        case ECodeForgeClassType::GameStateBase:
        case ECodeForgeClassType::PlayerController:
        case ECodeForgeClassType::PlayerState:
        case ECodeForgeClassType::HUD:
            return true;
        default:
            return false;
        }
    }

    /** Reserved Unreal/Blueprint identifiers that must not be used as names. */
    static const TArray<FString>& GetReservedNames()
    {
        static TArray<FString> Names = {
            TEXT("Class"), TEXT("Super"), TEXT("StaticClass")
        };
        return Names;
    }

    /** C++ keywords that must not be used as property or function names. */
    static const TArray<FString>& GetCppKeywords()
    {
        static TArray<FString> Keywords = {
            TEXT("delete"), TEXT("static"), TEXT("return"), TEXT("void"),
            TEXT("class"), TEXT("struct"), TEXT("enum"), TEXT("virtual"),
            TEXT("override"), TEXT("const"), TEXT("public"), TEXT("private"),
            TEXT("protected"), TEXT("new"), TEXT("template"), TEXT("typename"),
            TEXT("namespace"), TEXT("using"), TEXT("auto"), TEXT("register"),
            TEXT("break"), TEXT("continue"), TEXT("switch"), TEXT("case"),
            TEXT("default"), TEXT("goto"), TEXT("throw"), TEXT("try"), TEXT("catch")
        };
        return Keywords;
    }

    /** Helper: add an error result. */
    static FCodeForgeValidationResult MakeValidationError(const FString& Message, const FString& FieldName, const FString& AutoFixId = FString())
    {
        FCodeForgeValidationResult R;
        R.Severity = ECodeForgeValidationSeverity::Error;
        R.Message = Message;
        R.FieldName = FieldName;
        R.AutoFixId = AutoFixId;
        return R;
    }

    /** Helper: add a warning result. */
    static FCodeForgeValidationResult MakeValidationWarning(const FString& Message, const FString& FieldName, const FString& AutoFixId = FString())
    {
        FCodeForgeValidationResult R;
        R.Severity = ECodeForgeValidationSeverity::Warning;
        R.Message = Message;
        R.FieldName = FieldName;
        R.AutoFixId = AutoFixId;
        return R;
    }

    /**
     * Validate a single name against reserved names and C++ keywords.
     * Appends any errors to Results.
     */
    static void ValidateName(const FString& Name, const FString& Context,
        TArray<FCodeForgeValidationResult>& Results)
    {
        for (const FString& Reserved : GetReservedNames())
        {
            if (Name.Equals(Reserved, ESearchCase::CaseSensitive))
            {
                Results.Add(MakeValidationError(
                    FString::Printf(TEXT("'%s' uses reserved name '%s'"), *Context, *Name),
                    Context));
                return; // One error per name is enough
            }
        }
        for (const FString& Keyword : GetCppKeywords())
        {
            if (Name.Equals(Keyword, ESearchCase::CaseSensitive))
            {
                Results.Add(MakeValidationError(
                    FString::Printf(TEXT("'%s' uses C++ keyword '%s'"), *Context, *Name),
                    Context));
                return;
            }
        }
    }
} // namespace CodeForgeValidation

// ---------------------------------------------------------------------------
// UCodeForgeBlueprint — Validate()
// ---------------------------------------------------------------------------

TArray<FCodeForgeValidationResult> UCodeForgeBlueprint::Validate() const
{
    using namespace CodeForgeValidation;

    TArray<FCodeForgeValidationResult> Results;

    // ------------------------------------------------------------------
    // Rule 4: Missing class name
    // ------------------------------------------------------------------
    if (ClassName.IsEmpty())
    {
        Results.Add(MakeValidationError(TEXT("Class name is required"), TEXT("ClassName")));
    }

    // ------------------------------------------------------------------
    // Kind-specific validation
    // ------------------------------------------------------------------
    if (BlueprintKind == ECodeForgeBlueprintKind::Class)
    {
        // Collect all names (properties + functions + RPCs) for duplicate/reserved/keyword checks.
        TArray<FString> AllNames;

        // --- Properties ---
        for (const FCodeForgePropertyDef& Prop : Properties)
        {
            // Rule 1: duplicate names
            if (AllNames.Contains(Prop.Name))
            {
                Results.Add(MakeValidationError(
                    FString::Printf(TEXT("Duplicate name '%s' in class members"), *Prop.Name),
                    TEXT("Properties")));
            }
            else
            {
                AllNames.Add(Prop.Name);
            }

            // Rule 2: invalid specifier combo (bBlueprintReadWrite + bBlueprintReadOnly both true)
            if (Prop.bBlueprintReadWrite && Prop.bBlueprintReadOnly)
            {
                Results.Add(MakeValidationError(
                    FString::Printf(TEXT("Property '%s' has both BlueprintReadWrite and BlueprintReadOnly set"), *Prop.Name),
                    FString::Printf(TEXT("Properties.%s"), *Prop.Name)));
            }

            // Rule 5 & 6: reserved names and C++ keywords
            ValidateName(Prop.Name, FString::Printf(TEXT("Property '%s'"), *Prop.Name), Results);

            // Rule 8: RepNotify without Replicated
            if (Prop.bRepNotify && !Prop.bReplicated)
            {
                Results.Add(MakeValidationError(
                    FString::Printf(TEXT("Property '%s' has RepNotify set but Replicated is false"), *Prop.Name),
                    FString::Printf(TEXT("Properties.%s"), *Prop.Name),
                    FString::Printf(TEXT("EnableReplicated:%s"), *Prop.Name)));
            }

            // Rule 9: bool not prefixed with 'b'
            if (Prop.Type.Equals(TEXT("bool"), ESearchCase::CaseSensitive))
            {
                // Must start with 'b' followed by an uppercase letter
                bool bCorrectPrefix = Prop.Name.Len() >= 2
                    && Prop.Name[0] == TEXT('b')
                    && FChar::IsUpper(Prop.Name[1]);
                if (!bCorrectPrefix)
                {
                    Results.Add(MakeValidationWarning(
                        FString::Printf(TEXT("Bool property '%s' should be prefixed with 'b' followed by an uppercase letter"), *Prop.Name),
                        FString::Printf(TEXT("Properties.%s"), *Prop.Name),
                        FString::Printf(TEXT("FixBoolPrefix:%s"), *Prop.Name)));
                }
            }

            // Rule 11: replicated property on non-Actor class
            if (Prop.bReplicated && !IsActorDerived(ClassType))
            {
                Results.Add(MakeValidationWarning(
                    FString::Printf(TEXT("Property '%s' is replicated but class type '%s' is not Actor-derived"), *Prop.Name, *GetParentClassName()),
                    FString::Printf(TEXT("Properties.%s"), *Prop.Name)));
            }
        }

        // --- Functions ---
        for (const FCodeForgeFunctionDef& Func : Functions)
        {
            // Rule 1: duplicate names
            if (AllNames.Contains(Func.Name))
            {
                Results.Add(MakeValidationError(
                    FString::Printf(TEXT("Duplicate name '%s' in class members"), *Func.Name),
                    TEXT("Functions")));
            }
            else
            {
                AllNames.Add(Func.Name);
            }

            // Rule 5 & 6: reserved names and C++ keywords
            ValidateName(Func.Name, FString::Printf(TEXT("Function '%s'"), *Func.Name), Results);
        }

        // --- RPCs ---
        for (const FCodeForgeRPCDef& RPC : RPCs)
        {
            // Rule 1: duplicate names
            if (AllNames.Contains(RPC.Name))
            {
                Results.Add(MakeValidationError(
                    FString::Printf(TEXT("Duplicate name '%s' in class members"), *RPC.Name),
                    TEXT("RPCs")));
            }
            else
            {
                AllNames.Add(RPC.Name);
            }

            // Rule 5 & 6: reserved names and C++ keywords (RPC names)
            ValidateName(RPC.Name, FString::Printf(TEXT("RPC '%s'"), *RPC.Name), Results);
        }

        // Rule 3: RPC on non-Actor class
        if (RPCs.Num() > 0 && !IsActorDerived(ClassType))
        {
            Results.Add(MakeValidationError(
                FString::Printf(TEXT("RPCs are defined but class type '%s' is not Actor-derived"), *GetParentClassName()),
                TEXT("RPCs")));
        }

        // Rule 7: Unreliable RPC — validate each RPC is well-formed
        // (RPCs have no ReturnType by schema design; validate that RPC names are non-empty)
        for (const FCodeForgeRPCDef& RPC : RPCs)
        {
            if (RPC.Name.IsEmpty())
            {
                Results.Add(MakeValidationError(
                    TEXT("An RPC has an empty name"),
                    TEXT("RPCs")));
            }
        }

        // Rule 10: Empty class (BlueprintKind==Class, zero Properties + Functions + RPCs)
        if (Properties.Num() == 0 && Functions.Num() == 0 && RPCs.Num() == 0)
        {
            Results.Add(MakeValidationWarning(
                TEXT("Class has no properties, functions, or RPCs"),
                TEXT("Class")));
        }
    }
    else if (BlueprintKind == ECodeForgeBlueprintKind::Struct)
    {
        // --- Struct properties ---
        TArray<FString> StructNames;

        for (const FCodeForgePropertyDef& Prop : StructProperties)
        {
            // Rule 1: duplicate names within struct
            if (StructNames.Contains(Prop.Name))
            {
                Results.Add(MakeValidationError(
                    FString::Printf(TEXT("Duplicate name '%s' in struct properties"), *Prop.Name),
                    TEXT("StructProperties")));
            }
            else
            {
                StructNames.Add(Prop.Name);
            }

            // Rule 2: invalid specifier combo
            if (Prop.bBlueprintReadWrite && Prop.bBlueprintReadOnly)
            {
                Results.Add(MakeValidationError(
                    FString::Printf(TEXT("Struct property '%s' has both BlueprintReadWrite and BlueprintReadOnly set"), *Prop.Name),
                    FString::Printf(TEXT("StructProperties.%s"), *Prop.Name)));
            }

            // Rule 5 & 6: reserved names and C++ keywords
            ValidateName(Prop.Name, FString::Printf(TEXT("StructProperty '%s'"), *Prop.Name), Results);

            // Rule 9: bool not prefixed with 'b'
            if (Prop.Type.Equals(TEXT("bool"), ESearchCase::CaseSensitive))
            {
                bool bCorrectPrefix = Prop.Name.Len() >= 2
                    && Prop.Name[0] == TEXT('b')
                    && FChar::IsUpper(Prop.Name[1]);
                if (!bCorrectPrefix)
                {
                    Results.Add(MakeValidationWarning(
                        FString::Printf(TEXT("Bool struct property '%s' should be prefixed with 'b' followed by an uppercase letter"), *Prop.Name),
                        FString::Printf(TEXT("StructProperties.%s"), *Prop.Name),
                        FString::Printf(TEXT("FixBoolPrefix:%s"), *Prop.Name)));
                }
            }

            // Rule 12: replication specifiers on Struct property
            if (Prop.bReplicated)
            {
                Results.Add(MakeValidationError(
                    FString::Printf(TEXT("Struct property '%s' has replication flags set; structs cannot be replicated directly"), *Prop.Name),
                    FString::Printf(TEXT("StructProperties.%s"), *Prop.Name),
                    FString::Printf(TEXT("RemoveReplicationFlags:%s"), *Prop.Name)));
            }
        }
    }
    else if (BlueprintKind == ECodeForgeBlueprintKind::Interface)
    {
        TArray<FString> FunctionNames;

        for (const FCodeForgeFunctionDef& Func : Functions)
        {
            if (FunctionNames.Contains(Func.Name))
            {
                Results.Add(MakeValidationError(
                    FString::Printf(TEXT("Duplicate function name '%s' in interface"), *Func.Name),
                    TEXT("Functions")));
            }
            else
            {
                FunctionNames.Add(Func.Name);
            }

            if (Func.Name.IsEmpty())
            {
                Results.Add(MakeValidationError(
                    TEXT("An interface function has an empty name"),
                    TEXT("Functions")));
            }

            ValidateName(Func.Name, FString::Printf(TEXT("Interface function '%s'"), *Func.Name), Results);
        }

        if (Functions.Num() == 0)
        {
            Results.Add(MakeValidationWarning(
                TEXT("Interface has no functions"),
                TEXT("Interface")));
        }
    }
    // Enum kind: no special validation beyond ClassName (already checked above)

    return Results;
}

// ---------------------------------------------------------------------------
// UCodeForgeBlueprint — ApplyAutoFix()
// ---------------------------------------------------------------------------

void UCodeForgeBlueprint::ApplyAutoFix(const FString& AutoFixId)
{
    if (AutoFixId.StartsWith(TEXT("EnableReplicated:")))
    {
        // len("EnableReplicated:") == 17
        FString PropName = AutoFixId.RightChop(17);
        for (FCodeForgePropertyDef& Prop : Properties)
        {
            if (Prop.Name == PropName)
            {
                Prop.bReplicated = true;
                break;
            }
        }
    }
    else if (AutoFixId.StartsWith(TEXT("FixBoolPrefix:")))
    {
        // len("FixBoolPrefix:") == 14
        FString PropName = AutoFixId.RightChop(14);

        auto FixName = [&](TArray<FCodeForgePropertyDef>& Props)
        {
            for (FCodeForgePropertyDef& Prop : Props)
            {
                if (Prop.Name == PropName)
                {
                    FString NewName = TEXT("b") + Prop.Name;
                    // Capitalise the first letter after "b"
                    if (NewName.Len() > 1)
                    {
                        NewName[1] = FChar::ToUpper(NewName[1]);
                    }
                    Prop.Name = NewName;
                    break;
                }
            }
        };

        FixName(Properties);
        FixName(StructProperties);
    }
    else if (AutoFixId.StartsWith(TEXT("RemoveReplicationFlags:")))
    {
        // len("RemoveReplicationFlags:") == 23
        FString PropName = AutoFixId.RightChop(23);
        for (FCodeForgePropertyDef& Prop : StructProperties)
        {
            if (Prop.Name == PropName)
            {
                Prop.bReplicated = false;
                Prop.bRepNotify  = false;
                break;
            }
        }
    }

    // Mark the UObject as modified so the editor's undo/save system picks it up.
    // (Task 9 will handle deeper integration; this is safe to call in all contexts.)
    Modify();
}

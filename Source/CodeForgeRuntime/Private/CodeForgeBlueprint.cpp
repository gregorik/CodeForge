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

    // Visibility/Edit specifiers — mutually exclusive, first true wins
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

    // Exec is a standalone specifier — not combined with BlueprintCallable
    if (bExec)
    {
        Specifiers.Add(TEXT("Exec"));
    }
    // BlueprintPure implies BlueprintCallable — treat as mutually exclusive
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

    // Note: Replication is NOT a UCLASS specifier — it is enabled via
    // bReplicates = true in the constructor and GetLifetimeReplicatedProps().

    return FString::Join(Specifiers, TEXT(", "));
}

// ---------------------------------------------------------------------------
// UCodeForgeBlueprint — validation helpers
// ---------------------------------------------------------------------------

namespace CodeForgeValidation
{
    /** Returns true if this class type is Actor-derived. */
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

    /** Map of commonly miscapitalized types to their correct C++ / UE5 spelling. */
    static const TMap<FString, FString>& GetTypeCorrectionMap()
    {
        static TMap<FString, FString> Map;
        if (Map.Num() == 0)
        {
            // C++ primitives (wrong case → correct)
            Map.Add(TEXT("Float"),   TEXT("float"));
            Map.Add(TEXT("FLOAT"),   TEXT("float"));
            Map.Add(TEXT("Double"),  TEXT("double"));
            Map.Add(TEXT("DOUBLE"),  TEXT("double"));
            Map.Add(TEXT("Bool"),    TEXT("bool"));
            Map.Add(TEXT("BOOL"),    TEXT("bool"));
            Map.Add(TEXT("Int"),     TEXT("int"));
            Map.Add(TEXT("INT"),     TEXT("int"));
            Map.Add(TEXT("Int32"),   TEXT("int32"));
            Map.Add(TEXT("Int64"),   TEXT("int64"));
            Map.Add(TEXT("Uint8"),   TEXT("uint8"));
            Map.Add(TEXT("Uint32"), TEXT("uint32"));
            Map.Add(TEXT("Uint64"), TEXT("uint64"));

            // UE5 types (wrong case → correct)
            Map.Add(TEXT("fstring"),  TEXT("FString"));
            Map.Add(TEXT("Fstring"),  TEXT("FString"));
            Map.Add(TEXT("fname"),    TEXT("FName"));
            Map.Add(TEXT("Fname"),    TEXT("FName"));
            Map.Add(TEXT("ftext"),    TEXT("FText"));
            Map.Add(TEXT("Ftext"),    TEXT("FText"));
            Map.Add(TEXT("fvector"),  TEXT("FVector"));
            Map.Add(TEXT("Fvector"),  TEXT("FVector"));
            Map.Add(TEXT("frotator"), TEXT("FRotator"));
            Map.Add(TEXT("Frotator"), TEXT("FRotator"));
            Map.Add(TEXT("ftransform"), TEXT("FTransform"));
            Map.Add(TEXT("Ftransform"), TEXT("FTransform"));
            Map.Add(TEXT("fcolor"),   TEXT("FColor"));
            Map.Add(TEXT("Fcolor"),   TEXT("FColor"));
            Map.Add(TEXT("flinearcolor"), TEXT("FLinearColor"));
            Map.Add(TEXT("string"),   TEXT("FString"));
            Map.Add(TEXT("String"),   TEXT("FString"));
            Map.Add(TEXT("vector"),   TEXT("FVector"));
            Map.Add(TEXT("Vector"),   TEXT("FVector"));
        }
        return Map;
    }

    /**
     * Validate a type string. Checks for:
     *  - Empty type
     *  - Common miscapitalization (e.g. "Float" → "float")
     *  - Non-identifier characters
     * Appends warnings/errors to Results.
     */
    static void ValidateType(const FString& Type, const FString& Context, const FString& FieldName,
        TArray<FCodeForgeValidationResult>& Results)
    {
        if (Type.IsEmpty())
        {
            Results.Add(MakeValidationError(
                FString::Printf(TEXT("%s has an empty type"), *Context),
                FieldName));
            return;
        }

        // Check for common miscapitalization
        const FString* Correction = GetTypeCorrectionMap().Find(Type);
        if (Correction && !Type.Equals(*Correction, ESearchCase::CaseSensitive))
        {
            Results.Add(MakeValidationWarning(
                FString::Printf(TEXT("%s type '%s' should be '%s'"), *Context, *Type, **Correction),
                FieldName,
                FString::Printf(TEXT("FixType:%s:%s"), *FieldName, **Correction)));
        }

        // Check the base type name (strip TArray<>, TMap<>, pointer/ref markers)
        FString BaseType = Type;
        // Strip TArray< > wrapper
        if (BaseType.StartsWith(TEXT("TArray<")) && BaseType.EndsWith(TEXT(">")))
        {
            BaseType = BaseType.Mid(7, BaseType.Len() - 8).TrimStartAndEnd();
        }
        // Strip pointer/reference suffix
        BaseType.RemoveFromEnd(TEXT("*"));
        BaseType.RemoveFromEnd(TEXT("&"));
        BaseType = BaseType.TrimStartAndEnd();

        // Verify base type starts with a letter or underscore (valid C++ identifier start)
        if (BaseType.Len() > 0 && !FChar::IsAlpha(BaseType[0]) && BaseType[0] != TEXT('_'))
        {
            Results.Add(MakeValidationError(
                FString::Printf(TEXT("%s type '%s' is not a valid C++ type name"), *Context, *Type),
                FieldName));
        }
    }

    /**
     * Validate all parameters in a param list for type correctness.
     */
    static void ValidateParams(const TArray<FCodeForgeParamDef>& Params, const FString& OwnerContext, const FString& FieldName,
        TArray<FCodeForgeValidationResult>& Results)
    {
        for (const FCodeForgeParamDef& Param : Params)
        {
            if (Param.Name.IsEmpty())
            {
                Results.Add(MakeValidationError(
                    FString::Printf(TEXT("%s has a parameter with an empty name"), *OwnerContext),
                    FieldName));
            }
            ValidateType(Param.Type,
                FString::Printf(TEXT("%s parameter '%s'"), *OwnerContext, *Param.Name),
                FieldName, Results);
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
    // Rule 13: Class name prefix validation
    // ------------------------------------------------------------------
    if (!ClassName.IsEmpty() && ClassName.Len() >= 2)
    {
        TCHAR Prefix = ClassName[0];
        bool bSecondIsUpper = FChar::IsUpper(ClassName[1]);
        switch (BlueprintKind)
        {
        case ECodeForgeBlueprintKind::Class:
        {
            // Actor-derived classes must start with 'A', others with 'U'
            bool bExpectA = IsActorDerived(ClassType);
            TCHAR Expected = bExpectA ? TEXT('A') : TEXT('U');
            if (Prefix != Expected || !bSecondIsUpper)
            {
                Results.Add(MakeValidationWarning(
                    FString::Printf(TEXT("Class name '%s' should start with '%c' for %s-derived classes"),
                        *ClassName, Expected, bExpectA ? TEXT("Actor") : TEXT("UObject")),
                    TEXT("ClassName"),
                    FString::Printf(TEXT("FixClassPrefix:%c"), Expected)));
            }
            break;
        }
        case ECodeForgeBlueprintKind::Struct:
            if (Prefix != TEXT('F') || !bSecondIsUpper)
            {
                Results.Add(MakeValidationWarning(
                    FString::Printf(TEXT("Struct name '%s' should start with 'F'"), *ClassName),
                    TEXT("ClassName"),
                    TEXT("FixClassPrefix:F")));
            }
            break;
        case ECodeForgeBlueprintKind::Enum:
            if (Prefix != TEXT('E') || !bSecondIsUpper)
            {
                Results.Add(MakeValidationWarning(
                    FString::Printf(TEXT("Enum name '%s' should start with 'E'"), *ClassName),
                    TEXT("ClassName"),
                    TEXT("FixClassPrefix:E")));
            }
            break;
        
        }
    }

    // ------------------------------------------------------------------
    // Kind-specific validation
    // ------------------------------------------------------------------
    if (BlueprintKind == ECodeForgeBlueprintKind::Class)
    {
        // Collect all names (properties + functions) for duplicate/reserved/keyword checks.
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

            // Type validation
            ValidateType(Prop.Type,
                FString::Printf(TEXT("Property '%s'"), *Prop.Name),
                FString::Printf(TEXT("Properties.%s"), *Prop.Name), Results);

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

            // Type validation: return type
            if (!Func.ReturnType.IsEmpty() && !Func.ReturnType.Equals(TEXT("void"), ESearchCase::IgnoreCase))
            {
                ValidateType(Func.ReturnType,
                    FString::Printf(TEXT("Function '%s' return type"), *Func.Name),
                    FString::Printf(TEXT("Functions.%s"), *Func.Name), Results);
            }

            // Type validation: parameters
            ValidateParams(Func.Parameters,
                FString::Printf(TEXT("Function '%s'"), *Func.Name),
                FString::Printf(TEXT("Functions.%s"), *Func.Name), Results);
        }

        // Rule 10: Empty class (BlueprintKind==Class, zero members)
        if (Properties.Num() == 0 && Functions.Num() == 0)
        {
            Results.Add(MakeValidationWarning(
                TEXT("Class has no properties or functions"),
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

            // Type validation
            ValidateType(Prop.Type,
                FString::Printf(TEXT("Struct property '%s'"), *Prop.Name),
                FString::Printf(TEXT("StructProperties.%s"), *Prop.Name), Results);

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
    else if (BlueprintKind == ECodeForgeBlueprintKind::Enum)
    {
        // --- Enum entries ---
        TArray<FString> EntryNames;

        if (EnumEntries.Num() == 0)
        {
            Results.Add(MakeValidationWarning(
                TEXT("Enum has no entries"),
                TEXT("EnumEntries")));
        }

        for (const FCodeForgeEnumEntryDef& Entry : EnumEntries)
        {
            if (Entry.Name.IsEmpty())
            {
                Results.Add(MakeValidationError(
                    TEXT("An enum entry has an empty name"),
                    TEXT("EnumEntries")));
                continue;
            }

            // Duplicate entry names
            if (EntryNames.Contains(Entry.Name))
            {
                Results.Add(MakeValidationError(
                    FString::Printf(TEXT("Duplicate enum entry name '%s'"), *Entry.Name),
                    TEXT("EnumEntries")));
            }
            else
            {
                EntryNames.Add(Entry.Name);
            }

            // C++ keywords
            ValidateName(Entry.Name, FString::Printf(TEXT("Enum entry '%s'"), *Entry.Name), Results);

            // Entry name must start with a letter or underscore
            if (!FChar::IsAlpha(Entry.Name[0]) && Entry.Name[0] != TEXT('_'))
            {
                Results.Add(MakeValidationError(
                    FString::Printf(TEXT("Enum entry '%s' must start with a letter or underscore"), *Entry.Name),
                    TEXT("EnumEntries")));
            }
        }
    }


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

    else if (AutoFixId.StartsWith(TEXT("FixClassPrefix:")))
    {
        // Format: "FixClassPrefix:A" — prepend the correct prefix character
        // len("FixClassPrefix:") == 15
        FString PrefixChar = AutoFixId.RightChop(15);
        if (PrefixChar.Len() == 1 && !ClassName.IsEmpty())
        {
            // Strip any existing single-letter prefix if it's followed by an uppercase letter
            FString BaseName = ClassName;
            if (BaseName.Len() >= 2 && FChar::IsUpper(BaseName[1]))
            {
                BaseName = BaseName.RightChop(1);
            }
            ClassName = PrefixChar + BaseName;
        }
    }

    else if (AutoFixId.StartsWith(TEXT("FixType:")))
    {
        // Format: "FixType:FieldName:CorrectType"
        FString Remainder = AutoFixId.RightChop(8); // len("FixType:") == 8
        FString FieldName, CorrectType;
        if (Remainder.Split(TEXT(":"), &FieldName, &CorrectType))
        {
            // Try fixing in class properties
            for (FCodeForgePropertyDef& Prop : Properties)
            {
                if (FString::Printf(TEXT("Properties.%s"), *Prop.Name) == FieldName)
                {
                    Prop.Type = CorrectType;
                    break;
                }
            }
            // Try fixing in struct properties
            for (FCodeForgePropertyDef& Prop : StructProperties)
            {
                if (FString::Printf(TEXT("StructProperties.%s"), *Prop.Name) == FieldName)
                {
                    Prop.Type = CorrectType;
                    break;
                }
            }
        }
    }

    // Mark the UObject as modified so the editor's undo/save system picks it up.
    Modify();
}



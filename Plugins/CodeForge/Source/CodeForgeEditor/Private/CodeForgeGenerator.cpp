// CodeForgeGenerator.cpp
// Implements FCodeForgeGenerator — the orchestration layer that reads a
// UCodeForgeBlueprint schema, populates a template context, runs templates
// through FCodeForgeTemplateEngine, and produces .h/.cpp file content.

#include "CodeForgeGenerator.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgePropertyDef.h"
#include "CodeForgeFunctionDef.h"
#include "CodeForgeRPCDef.h"
#include "CodeForgeEnumDef.h"
#include "CodeForgeTypes.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

// ---------------------------------------------------------------------------
// SetTemplatePath
// ---------------------------------------------------------------------------

void FCodeForgeGenerator::SetTemplatePath(const FString& Path)
{
    TemplatePath = Path;
}

// ---------------------------------------------------------------------------
// Generate
// ---------------------------------------------------------------------------

FCodeForgeGeneratorResult FCodeForgeGenerator::Generate(const UCodeForgeBlueprint* Blueprint) const
{
    FCodeForgeGeneratorResult Result;

    if (!Blueprint)
    {
        Result.ErrorMessage = TEXT("Blueprint is null.");
        return Result;
    }

    // 1. Validate the schema
    TArray<FCodeForgeValidationResult> ValidationResults = Blueprint->Validate();
    for (const FCodeForgeValidationResult& VR : ValidationResults)
    {
        if (VR.Severity == ECodeForgeValidationSeverity::Error)
        {
            Result.ErrorMessage = FString::Printf(
                TEXT("Validation failed: %s"), *VR.Message);
            return Result;
        }
    }

    // 2. Build the template context
    FCodeForgeTemplateContext Context = BuildContext(Blueprint);

    // 3. Create the template engine and set up the resolver
    FCodeForgeTemplateEngine Engine;
    Engine.SetTemplateResolver([this](const FString& Name) -> FString
    {
        return LoadTemplate(Name);
    });

    // 4. Generate header
    {
        FString HeaderTemplateName = SelectTemplate(Blueprint, /*bHeader=*/true);
        if (HeaderTemplateName.IsEmpty())
        {
            Result.ErrorMessage = TEXT("No header template found for this Blueprint kind.");
            return Result;
        }

        FString HeaderTemplateContent = LoadTemplate(HeaderTemplateName);
        if (HeaderTemplateContent.IsEmpty())
        {
            Result.ErrorMessage = FString::Printf(
                TEXT("Could not load header template: %s"), *HeaderTemplateName);
            return Result;
        }

        FCodeForgeTemplateResult HeaderResult = Engine.Process(HeaderTemplateContent, Context);
        if (!HeaderResult.bSuccess)
        {
            Result.ErrorMessage = FString::Printf(
                TEXT("Header template error: %s"), *HeaderResult.ErrorMessage);
            return Result;
        }
        Result.HeaderContent = HeaderResult.Output;
    }

    // 5. Generate source (Enum has no source file)
    {
        FString SourceTemplateName = SelectTemplate(Blueprint, /*bHeader=*/false);
        if (!SourceTemplateName.IsEmpty())
        {
            FString SourceTemplateContent = LoadTemplate(SourceTemplateName);
            if (SourceTemplateContent.IsEmpty())
            {
                Result.ErrorMessage = FString::Printf(
                    TEXT("Could not load source template: %s"), *SourceTemplateName);
                return Result;
            }

            FCodeForgeTemplateResult SourceResult = Engine.Process(SourceTemplateContent, Context);
            if (!SourceResult.bSuccess)
            {
                Result.ErrorMessage = FString::Printf(
                    TEXT("Source template error: %s"), *SourceResult.ErrorMessage);
                return Result;
            }
            Result.SourceContent = SourceResult.Output;
        }
    }

    // 6. Populate paths (caller can override with actual directories via WriteToProject)
    Result.HeaderPath = Blueprint->ClassName + TEXT(".h");
    Result.SourcePath  = Blueprint->ClassName + TEXT(".cpp");

    Result.bSuccess = true;
    return Result;
}

// ---------------------------------------------------------------------------
// WriteToProject
// ---------------------------------------------------------------------------

bool FCodeForgeGenerator::WriteToProject(
    const FCodeForgeGeneratorResult& Result,
    const FString& PublicDir, const FString& PrivateDir,
    const FString& SubDir)
{
    if (!Result.bSuccess)
    {
        return false;
    }

    // Derive the class name from the stored path
    FString ClassName = FPaths::GetBaseFilename(Result.HeaderPath);

    // Build full output paths
    FString HeaderPath = FPaths::Combine(PublicDir,  SubDir, ClassName + TEXT(".h"));
    FString SourcePath  = FPaths::Combine(PrivateDir, SubDir, ClassName + TEXT(".cpp"));

    // Normalize and ensure directories exist
    FPaths::NormalizePath(HeaderPath);
    FPaths::NormalizePath(SourcePath);

    IFileManager& FM = IFileManager::Get();
    FM.MakeDirectory(*FPaths::GetPath(HeaderPath), /*Tree=*/true);
    FM.MakeDirectory(*FPaths::GetPath(SourcePath),  /*Tree=*/true);

    // Write files
    if (!FFileHelper::SaveStringToFile(Result.HeaderContent, *HeaderPath))
    {
        return false;
    }

    if (!Result.SourceContent.IsEmpty())
    {
        if (!FFileHelper::SaveStringToFile(Result.SourceContent, *SourcePath))
        {
            return false;
        }
    }

    return true;
}

// ---------------------------------------------------------------------------
// BuildContext
// ---------------------------------------------------------------------------

FCodeForgeTemplateContext FCodeForgeGenerator::BuildContext(const UCodeForgeBlueprint* Blueprint) const
{
    FCodeForgeTemplateContext Context;

    // Derive MODULE_API macro from module target name (uppercase + _API)
    FString ModuleAPI;
    if (!Blueprint->ModuleTarget.IsEmpty())
    {
        ModuleAPI = Blueprint->ModuleTarget.ToUpper() + TEXT("_API");
    }

    // -----------------------------------------------------------------------
    // Common variables (all kinds)
    // -----------------------------------------------------------------------
    Context.Variables.Add(TEXT("ClassName"),  Blueprint->ClassName);
    Context.Variables.Add(TEXT("ModuleAPI"),  ModuleAPI);

    // -----------------------------------------------------------------------
    // Kind-specific context population
    // -----------------------------------------------------------------------
    switch (Blueprint->BlueprintKind)
    {
    case ECodeForgeBlueprintKind::Class:
    {
        // Class-level variables
        Context.Variables.Add(TEXT("ParentClassName"), Blueprint->GetParentClassName());
        Context.Variables.Add(TEXT("ParentInclude"),   Blueprint->GetParentIncludePath());
        Context.Variables.Add(TEXT("ClassSpecifiers"), Blueprint->GetClassSpecifiers());
        Context.Variables.Add(TEXT("OwnerClassName"),  Blueprint->ClassName);

        // Conditions
        Context.Conditions.Add(TEXT("Replicated"), Blueprint->bReplicated);

        // Check for RepNotify properties
        bool bHasRepNotify = false;
        for (const FCodeForgePropertyDef& Prop : Blueprint->Properties)
        {
            if (Prop.bRepNotify)
            {
                bHasRepNotify = true;
                break;
            }
        }
        Context.Conditions.Add(TEXT("HasRepNotifyProperties"), bHasRepNotify);

        // Check for NativeEvent functions
        bool bHasNativeEvents = false;
        for (const FCodeForgeFunctionDef& Func : Blueprint->Functions)
        {
            if (Func.bBlueprintNativeEvent)
            {
                bHasNativeEvents = true;
                break;
            }
        }
        Context.Conditions.Add(TEXT("HasNativeEventFunctions"), bHasNativeEvents);

        // ---- Properties array ----
        TArray<FCodeForgeTemplateArrayItem> PropertiesArray;
        TArray<FCodeForgeTemplateArrayItem> RepNotifyArray;
        TArray<FCodeForgeTemplateArrayItem> ReplicatedArray;

        for (const FCodeForgePropertyDef& Prop : Blueprint->Properties)
        {
            // All properties
            FCodeForgeTemplateArrayItem Item;
            Item.Values.Add(TEXT("PropertySpecifiers"), Prop.BuildSpecifierString());
            Item.Values.Add(TEXT("PropertyType"),       Prop.Type);
            Item.Values.Add(TEXT("PropertyName"),       Prop.Name);
            Item.Values.Add(TEXT("PropertyDefault"),    Prop.BuildDefaultValueSuffix());
            PropertiesArray.Add(Item);

            // RepNotify filtered set
            if (Prop.bRepNotify)
            {
                FCodeForgeTemplateArrayItem RepNotifyItem;
                RepNotifyItem.Values.Add(TEXT("PropertyName"),   Prop.Name);
                RepNotifyItem.Values.Add(TEXT("OwnerClassName"), Blueprint->ClassName);
                RepNotifyArray.Add(RepNotifyItem);
            }

            // Replicated filtered set
            if (Prop.bReplicated)
            {
                FCodeForgeTemplateArrayItem ReplicatedItem;
                ReplicatedItem.Values.Add(TEXT("PropertyName"),        Prop.Name);
                ReplicatedItem.Values.Add(TEXT("OwnerClassName"),      Blueprint->ClassName);
                ReplicatedItem.Values.Add(TEXT("ReplicationCondition"),
                    RepConditionToString(Prop.ReplicationCondition));
                ReplicatedArray.Add(ReplicatedItem);
            }
        }

        Context.Arrays.Add(TEXT("Properties"),           PropertiesArray);
        Context.Arrays.Add(TEXT("RepNotifyProperties"),  RepNotifyArray);
        Context.Arrays.Add(TEXT("ReplicatedProperties"), ReplicatedArray);

        // ---- Functions array ----
        TArray<FCodeForgeTemplateArrayItem> FunctionsArray;
        TArray<FCodeForgeTemplateArrayItem> NativeEventArray;

        for (const FCodeForgeFunctionDef& Func : Blueprint->Functions)
        {
            FCodeForgeTemplateArrayItem Item;
            Item.Values.Add(TEXT("FunctionSpecifiers"), Func.BuildSpecifierString());
            Item.Values.Add(TEXT("FunctionReturnType"), Func.ReturnType);
            Item.Values.Add(TEXT("FunctionName"),       Func.Name);
            Item.Values.Add(TEXT("FunctionParams"),     Func.BuildParamListString());
            FunctionsArray.Add(Item);

            if (Func.bBlueprintNativeEvent)
            {
                FCodeForgeTemplateArrayItem NEItem;
                NEItem.Values.Add(TEXT("FunctionReturnType"), Func.ReturnType);
                NEItem.Values.Add(TEXT("FunctionName"),       Func.Name);
                NEItem.Values.Add(TEXT("FunctionParams"),     Func.BuildParamListString());
                NEItem.Values.Add(TEXT("OwnerClassName"),     Blueprint->ClassName);
                // Provide a return statement for non-void functions.
                // Template engine merges item Values into Variables per iteration,
                // but does NOT support per-item Conditions.  We encode the return
                // statement directly as a variable value so the template can embed it.
                bool bReturnsVoid = Func.ReturnType.TrimStartAndEnd().Equals(
                    TEXT("void"), ESearchCase::IgnoreCase);
                NEItem.Values.Add(TEXT("FunctionReturnStatement"),
                    bReturnsVoid ? TEXT("") :
                    TEXT("\n    return ") + Func.ReturnType + TEXT("();"));
                NativeEventArray.Add(NEItem);
            }
        }

        Context.Arrays.Add(TEXT("Functions"),            FunctionsArray);
        Context.Arrays.Add(TEXT("NativeEventFunctions"), NativeEventArray);

        // ---- RPCs array ----
        TArray<FCodeForgeTemplateArrayItem> RPCsArray;
        for (const FCodeForgeRPCDef& RPC : Blueprint->RPCs)
        {
            FCodeForgeTemplateArrayItem Item;
            Item.Values.Add(TEXT("RPCSpecifiers"), RPC.BuildSpecifierString());
            Item.Values.Add(TEXT("RPCName"),       RPC.Name);
            Item.Values.Add(TEXT("RPCParams"),     RPC.BuildParamListString());
            Item.Values.Add(TEXT("OwnerClassName"), Blueprint->ClassName);
            RPCsArray.Add(Item);
        }
        Context.Arrays.Add(TEXT("RPCs"), RPCsArray);

        break;
    }

    case ECodeForgeBlueprintKind::Struct:
    {
        // Struct specifiers
        FString StructSpecifiers = Blueprint->bBlueprintType
            ? TEXT("BlueprintType") : TEXT("");
        Context.Variables.Add(TEXT("StructSpecifiers"), StructSpecifiers);

        // Properties from StructProperties
        TArray<FCodeForgeTemplateArrayItem> PropertiesArray;
        for (const FCodeForgePropertyDef& Prop : Blueprint->StructProperties)
        {
            FCodeForgeTemplateArrayItem Item;
            Item.Values.Add(TEXT("PropertySpecifiers"), Prop.BuildSpecifierString());
            Item.Values.Add(TEXT("PropertyType"),       Prop.Type);
            Item.Values.Add(TEXT("PropertyName"),       Prop.Name);
            Item.Values.Add(TEXT("PropertyDefault"),    Prop.BuildDefaultValueSuffix());
            PropertiesArray.Add(Item);
        }
        Context.Arrays.Add(TEXT("Properties"), PropertiesArray);

        break;
    }

    case ECodeForgeBlueprintKind::Enum:
    {
        // Enum specifiers
        FString EnumSpecifiers = Blueprint->bBlueprintType
            ? TEXT("BlueprintType") : TEXT("");
        Context.Variables.Add(TEXT("EnumSpecifiers"), EnumSpecifiers);

        // EnumEntries array
        TArray<FCodeForgeTemplateArrayItem> EntriesArray;
        for (const FCodeForgeEnumEntryDef& Entry : Blueprint->EnumEntries)
        {
            FCodeForgeTemplateArrayItem Item;
            Item.Values.Add(TEXT("EntryName"), Entry.Name);

            // " = N" if explicit value, else ""
            FString EntryValue;
            if (Entry.bHasExplicitValue)
            {
                EntryValue = FString::Printf(TEXT(" = %lld"), Entry.ExplicitValue);
            }
            Item.Values.Add(TEXT("EntryValue"), EntryValue);

            // UMETA: DisplayName and Hidden
            FString EntryMeta;
            if (!Entry.DisplayName.IsEmpty())
            {
                EntryMeta = FString::Printf(TEXT("DisplayName=\"%s\""), *Entry.DisplayName);
            }
            if (Entry.bHidden)
            {
                if (!EntryMeta.IsEmpty())
                {
                    EntryMeta += TEXT(", ");
                }
                EntryMeta += TEXT("Hidden");
            }
            Item.Values.Add(TEXT("EntryMeta"), EntryMeta);

            EntriesArray.Add(Item);
        }
        Context.Arrays.Add(TEXT("EnumEntries"), EntriesArray);

        break;
    }
    }

    return Context;
}

// ---------------------------------------------------------------------------
// SelectTemplate
// ---------------------------------------------------------------------------

FString FCodeForgeGenerator::SelectTemplate(
    const UCodeForgeBlueprint* Blueprint, bool bHeader) const
{
    switch (Blueprint->BlueprintKind)
    {
    case ECodeForgeBlueprintKind::Class:
    {
        bool bIsActorFamily = false;
        bool bIsComponent   = false;

        switch (Blueprint->ClassType)
        {
        case ECodeForgeClassType::Actor:
        case ECodeForgeClassType::Pawn:
        case ECodeForgeClassType::Character:
        case ECodeForgeClassType::GameModeBase:
        case ECodeForgeClassType::GameStateBase:
        case ECodeForgeClassType::PlayerController:
        case ECodeForgeClassType::PlayerState:
        case ECodeForgeClassType::HUD:
            bIsActorFamily = true;
            break;

        case ECodeForgeClassType::ActorComponent:
        case ECodeForgeClassType::SceneComponent:
            bIsComponent = true;
            break;

        case ECodeForgeClassType::Object:
        default:
            break;
        }

        if (bIsActorFamily)
        {
            return bHeader ? TEXT("actor_header") : TEXT("actor_source");
        }
        else if (bIsComponent)
        {
            return bHeader ? TEXT("component_header") : TEXT("component_source");
        }
        else
        {
            // Object
            return bHeader ? TEXT("object_header") : TEXT("object_source");
        }
    }

    case ECodeForgeBlueprintKind::Struct:
        return bHeader ? TEXT("struct_header") : TEXT("struct_source");

    case ECodeForgeBlueprintKind::Enum:
        // Enums have a header only — no source file
        return bHeader ? TEXT("enum_header") : TEXT("");
    }

    return TEXT("");
}

// ---------------------------------------------------------------------------
// LoadTemplate
// ---------------------------------------------------------------------------

FString FCodeForgeGenerator::LoadTemplate(const FString& TemplateName) const
{
    // Resolve root search path
    FString SearchRoot = TemplatePath;
    if (SearchRoot.IsEmpty())
    {
        // Default: plugin Content/Templates directory
        SearchRoot = FPaths::Combine(
            FPaths::ProjectPluginsDir(), TEXT("CodeForge/Content/Templates"));
    }

    // Sub-directories to search (order matters — prefer exact match in common first)
    static const TCHAR* SubDirs[] = {
        TEXT("Common"),
        TEXT("Class"),
        TEXT("Struct"),
        TEXT("Enum"),
        TEXT("Replication")
    };

    for (const TCHAR* SubDir : SubDirs)
    {
        FString CandidatePath = FPaths::Combine(
            SearchRoot, SubDir, TemplateName + TEXT(".cft"));
        FPaths::NormalizePath(CandidatePath);

        FString Content;
        if (FFileHelper::LoadFileToString(Content, *CandidatePath))
        {
            return Content;
        }
    }

    // Not found in any sub-directory — return empty (engine will error)
    return TEXT("");
}

// ---------------------------------------------------------------------------
// RepConditionToString
// ---------------------------------------------------------------------------

FString FCodeForgeGenerator::RepConditionToString(ECodeForgeRepCondition Condition)
{
    switch (Condition)
    {
    case ECodeForgeRepCondition::None:                      return TEXT("COND_None");
    case ECodeForgeRepCondition::InitialOnly:               return TEXT("COND_InitialOnly");
    case ECodeForgeRepCondition::OwnerOnly:                 return TEXT("COND_OwnerOnly");
    case ECodeForgeRepCondition::SkipOwner:                 return TEXT("COND_SkipOwner");
    case ECodeForgeRepCondition::SimulatedOnly:             return TEXT("COND_SimulatedOnly");
    case ECodeForgeRepCondition::AutonomousOnly:            return TEXT("COND_AutonomousOnly");
    case ECodeForgeRepCondition::SimulatedOrPhysics:        return TEXT("COND_SimulatedOrPhysics");
    case ECodeForgeRepCondition::InitialOrOwner:            return TEXT("COND_InitialOrOwner");
    case ECodeForgeRepCondition::Custom:                    return TEXT("COND_Custom");
    case ECodeForgeRepCondition::ReplayOrOwner:             return TEXT("COND_ReplayOrOwner");
    case ECodeForgeRepCondition::ReplayOnly:                return TEXT("COND_ReplayOnly");
    case ECodeForgeRepCondition::SimulatedOnlyNoReplay:     return TEXT("COND_SimulatedOnlyNoReplay");
    case ECodeForgeRepCondition::SimulatedOrPhysicsNoReplay:return TEXT("COND_SimulatedOrPhysicsNoReplay");
    case ECodeForgeRepCondition::SkipReplay:                return TEXT("COND_SkipReplay");
    case ECodeForgeRepCondition::Dynamic:                   return TEXT("COND_Dynamic");
    case ECodeForgeRepCondition::Never:                     return TEXT("COND_Never");
    default:                                                return TEXT("COND_None");
    }
}

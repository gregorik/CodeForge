// CodeForgeGenerator.cpp
// Implements FCodeForgeGenerator — the orchestration layer that reads a
// UCodeForgeBlueprint schema, populates a template context, runs templates
// through FCodeForgeTemplateEngine, and produces .h/.cpp file content.

#include "CodeForgeGenerator.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgeModuleScanner.h"
#include "CodeForgePropertyDef.h"
#include "CodeForgeFunctionDef.h"
#include "CodeForgeEnumDef.h"
#include "CodeForgeTypes.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "CodeForgeSettings.h"

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
    const FString& SubDir) const
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
    FPaths::NormalizeFilename(HeaderPath);
    FPaths::NormalizeFilename(SourcePath);

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
            IFileManager::Get().Delete(*HeaderPath);
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
        // Determine Unreal class prefix (A for Actor-family, U for UObject-family)
        TCHAR ExpectedPrefix;
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
            ExpectedPrefix = TEXT('A');
            break;
        default:
            ExpectedPrefix = TEXT('U');
            break;
        }

        // If ClassName already starts with the expected prefix (e.g. "ARPGCharacter"),
        // don't double it. Convention: prefix char + uppercase letter.
        bool bAlreadyHasPrefix = Blueprint->ClassName.Len() >= 2
            && Blueprint->ClassName[0] == ExpectedPrefix
            && FChar::IsUpper(Blueprint->ClassName[1]);

        FString ClassPrefix = bAlreadyHasPrefix ? TEXT("") : FString(1, &ExpectedPrefix);
        Context.Variables.Add(TEXT("ClassPrefix"), ClassPrefix);
        FString PrefixedClassName = ClassPrefix + Blueprint->ClassName;

        // Class-level variables
        Context.Variables.Add(TEXT("ParentClassName"), Blueprint->GetParentClassName());
        Context.Variables.Add(TEXT("ParentInclude"),   Blueprint->GetParentIncludePath());
        Context.Variables.Add(TEXT("ClassSpecifiers"), Blueprint->GetClassSpecifiers());
        Context.Variables.Add(TEXT("OwnerClassName"),  PrefixedClassName);

        // Conditions
        Context.Conditions.Add(TEXT("Replicated"), Blueprint->bReplicated);

        // ---- Constructor body ----
        Context.Variables.Add(TEXT("ConstructorBody"), Blueprint->ConstructorBody);

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
            if (Prop.bRepNotify && Prop.bReplicated)
            {
                FCodeForgeTemplateArrayItem RepNotifyItem;
                RepNotifyItem.Values.Add(TEXT("PropertyName"),   Prop.Name);
                RepNotifyItem.Values.Add(TEXT("OwnerClassName"), PrefixedClassName);
                // OnRepBody: custom body or fallback TODO
                RepNotifyItem.Values.Add(TEXT("OnRepBody"),
                    Prop.OnRepBody.IsEmpty()
                        ? TEXT("\t// TODO: Implement replication notify logic")
                        : Prop.OnRepBody);
                RepNotifyArray.Add(RepNotifyItem);
            }

            // Replicated filtered set
            if (Prop.bReplicated)
            {
                FCodeForgeTemplateArrayItem ReplicatedItem;
                ReplicatedItem.Values.Add(TEXT("PropertyName"),        Prop.Name);
                ReplicatedItem.Values.Add(TEXT("OwnerClassName"),      PrefixedClassName);
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
        TArray<FCodeForgeTemplateArrayItem> ImplementedFunctionsArray;

        for (const FCodeForgeFunctionDef& Func : Blueprint->Functions)
        {
            FCodeForgeTemplateArrayItem Item;
            Item.Values.Add(TEXT("FunctionSpecifiers"), Func.BuildSpecifierString());
            Item.Values.Add(TEXT("FunctionReturnType"), Func.ReturnType);
            Item.Values.Add(TEXT("FunctionName"),       Func.Name);
            Item.Values.Add(TEXT("FunctionParams"),     Func.BuildParamListString());
            Item.Values.Add(TEXT("ConstQualifier"),     Func.bConst ? TEXT(" const") : TEXT(""));
            FunctionsArray.Add(Item);

            if (Func.bBlueprintNativeEvent)
            {
                FCodeForgeTemplateArrayItem NEItem;
                NEItem.Values.Add(TEXT("FunctionReturnType"), Func.ReturnType);
                NEItem.Values.Add(TEXT("FunctionName"),       Func.Name);
                NEItem.Values.Add(TEXT("FunctionParams"),     Func.BuildParamListString());
                NEItem.Values.Add(TEXT("ConstQualifier"),     Func.bConst ? TEXT(" const") : TEXT(""));
                NEItem.Values.Add(TEXT("OwnerClassName"),     PrefixedClassName);

                bool bReturnsVoid = Func.ReturnType.TrimStartAndEnd().Equals(
                    TEXT("void"), ESearchCase::IgnoreCase);
                NEItem.Values.Add(TEXT("FunctionReturnStatement"),
                    bReturnsVoid ? TEXT("") :
                    TEXT("    return {};"));

                // FunctionBody: custom body or fallback TODO
                if (!Func.FunctionBody.IsEmpty())
                {
                    NEItem.Values.Add(TEXT("FunctionBody"), Func.FunctionBody);
                }
                else
                {
                    FString DefaultBody = FString::Printf(TEXT("\t// TODO: Implement %s logic"), *Func.Name);
                    if (!bReturnsVoid)
                    {
                        DefaultBody += TEXT("\n\treturn {};");
                    }
                    NEItem.Values.Add(TEXT("FunctionBody"), DefaultBody);
                }
                NativeEventArray.Add(NEItem);
            }
            else if (!Func.FunctionBody.IsEmpty())
            {
                // Non-NativeEvent with a body → emit implementation in source file
                FCodeForgeTemplateArrayItem ImplItem;
                ImplItem.Values.Add(TEXT("FunctionReturnType"), Func.ReturnType);
                ImplItem.Values.Add(TEXT("FunctionName"),       Func.Name);
                ImplItem.Values.Add(TEXT("FunctionParams"),     Func.BuildParamListString());
                ImplItem.Values.Add(TEXT("ConstQualifier"),     Func.bConst ? TEXT(" const") : TEXT(""));
                ImplItem.Values.Add(TEXT("OwnerClassName"),     PrefixedClassName);
                ImplItem.Values.Add(TEXT("FunctionBody"),       Func.FunctionBody);
                ImplementedFunctionsArray.Add(ImplItem);
            }
        }

        Context.Arrays.Add(TEXT("Functions"),            FunctionsArray);
        Context.Arrays.Add(TEXT("NativeEventFunctions"), NativeEventArray);
        Context.Arrays.Add(TEXT("ImplementedFunctions"), ImplementedFunctionsArray);

        // ---- Additional includes for custom types ----
        {
            TSet<FString> UniqueIncludes;
            for (const FCodeForgePropertyDef& Prop : Blueprint->Properties)
            {
                if (!Prop.IncludePath.IsEmpty())
                {
                    UniqueIncludes.Add(Prop.IncludePath);
                }
            }
            for (const FCodeForgeFunctionDef& Func : Blueprint->Functions)
            {
                if (!Func.IncludePath.IsEmpty())
                {
                    UniqueIncludes.Add(Func.IncludePath);
                }
            }

            TArray<FCodeForgeTemplateArrayItem> IncludesArray;
            for (const FString& Inc : UniqueIncludes)
            {
                FCodeForgeTemplateArrayItem Item;
                Item.Values.Add(TEXT("IncludePath"), Inc);
                IncludesArray.Add(Item);
            }
            Context.Arrays.Add(TEXT("AdditionalIncludes"), IncludesArray);
        }

        break;
    }

    case ECodeForgeBlueprintKind::Struct:
    {
        // Add F prefix only if ClassName doesn't already have it
        {
            bool bAlreadyHasPrefix = Blueprint->ClassName.Len() >= 2
                && Blueprint->ClassName[0] == TEXT('F')
                && FChar::IsUpper(Blueprint->ClassName[1]);
            Context.Variables.Add(TEXT("ClassPrefix"),
                bAlreadyHasPrefix ? TEXT("") : TEXT("F"));
        }

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

        // ---- Additional includes for custom types ----
        {
            TSet<FString> UniqueIncludes;
            for (const FCodeForgePropertyDef& Prop : Blueprint->StructProperties)
            {
                if (!Prop.IncludePath.IsEmpty())
                {
                    UniqueIncludes.Add(Prop.IncludePath);
                }
            }

            TArray<FCodeForgeTemplateArrayItem> IncludesArray;
            for (const FString& Inc : UniqueIncludes)
            {
                FCodeForgeTemplateArrayItem Item;
                Item.Values.Add(TEXT("IncludePath"), Inc);
                IncludesArray.Add(Item);
            }
            Context.Arrays.Add(TEXT("AdditionalIncludes"), IncludesArray);
        }

        break;
    }

    case ECodeForgeBlueprintKind::Enum:
    {
        // Add E prefix only if ClassName doesn't already have it
        {
            bool bAlreadyHasPrefix = Blueprint->ClassName.Len() >= 2
                && Blueprint->ClassName[0] == TEXT('E')
                && FChar::IsUpper(Blueprint->ClassName[1]);
            Context.Variables.Add(TEXT("ClassPrefix"),
                bAlreadyHasPrefix ? TEXT("") : TEXT("E"));
        }

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
            // Build complete UMETA suffix — empty string when no meta is set
            FString EntryMetaSuffix;
            if (!EntryMeta.IsEmpty())
            {
                EntryMetaSuffix = FString::Printf(TEXT(" UMETA(%s)"), *EntryMeta);
            }
            Item.Values.Add(TEXT("EntryMetaSuffix"), EntryMetaSuffix);

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
        // Check settings for custom template path
        const UCodeForgeSettings* Settings = GetDefault<UCodeForgeSettings>();
        if (Settings && !Settings->CustomTemplatePath.Path.IsEmpty())
        {
            SearchRoot = Settings->CustomTemplatePath.Path;
        }
    }
    if (SearchRoot.IsEmpty())
    {
        // Default: plugin Content/Templates directory — use IPluginManager for robust path resolution
        TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("CodeForge"));
        if (Plugin.IsValid())
        {
            SearchRoot = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Content/Templates"));
        }
        else
        {
            // Fallback to well-known path
            SearchRoot = FPaths::Combine(
                FPaths::ProjectPluginsDir(), TEXT("CodeForge/Content/Templates"));
        }
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
        FPaths::NormalizeFilename(CandidatePath);

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

// ---------------------------------------------------------------------------
// GenerateAndIntegrate
// ---------------------------------------------------------------------------

FCodeForgeGeneratorResult FCodeForgeGenerator::GenerateAndIntegrate(
    const UCodeForgeBlueprint* Blueprint, const FString& ProjectDir)
{
    // 1. Generate header + source content
    FCodeForgeGeneratorResult Result = Generate(Blueprint);
    if (!Result.bSuccess)
    {
        return Result;
    }

    // 2. Locate the target module via module scanner
    FCodeForgeModuleScanner Scanner;
    TArray<FCodeForgeModuleInfo> Modules = Scanner.ScanProject(ProjectDir);

    FString PublicDir;
    FString PrivateDir;

    for (const FCodeForgeModuleInfo& Module : Modules)
    {
        if (Module.ModuleName == Blueprint->ModuleTarget)
        {
            PublicDir  = Module.PublicDir;
            PrivateDir = Module.PrivateDir;
            break;
        }
    }

    if (PublicDir.IsEmpty() || PrivateDir.IsEmpty())
    {
        Result.bSuccess = false;
        Result.ErrorMessage = FString::Printf(
            TEXT("Could not find module '%s' in project '%s'."),
            *Blueprint->ModuleTarget, *ProjectDir);
        return Result;
    }

    // 3. Write generated files to the project
    if (!WriteToProject(Result, PublicDir, PrivateDir, Blueprint->SubDirectory))
    {
        Result.bSuccess = false;
        Result.ErrorMessage = TEXT("Failed to write generated files to project.");
        return Result;
    }

    // 4. Build manifest entry from the blueprint
    FCodeForgeManifestEntry ManifestEntry;
    ManifestEntry.ClassName = Blueprint->ClassName;

    if (Blueprint->BlueprintKind == ECodeForgeBlueprintKind::Class)
    {
        ManifestEntry.ParentClassName = Blueprint->GetParentClassName();

        for (const FCodeForgePropertyDef& Prop : Blueprint->Properties)
        {
            ManifestEntry.PropertyNames.Add(Prop.Name);
            ManifestEntry.PropertyTypes.Add(Prop.Type);
        }

        for (const FCodeForgeFunctionDef& Func : Blueprint->Functions)
        {
            ManifestEntry.FunctionNames.Add(Func.Name);
        }
    }
    else if (Blueprint->BlueprintKind == ECodeForgeBlueprintKind::Struct)
    {
        for (const FCodeForgePropertyDef& Prop : Blueprint->StructProperties)
        {
            ManifestEntry.PropertyNames.Add(Prop.Name);
            ManifestEntry.PropertyTypes.Add(Prop.Type);
        }
    }

    // 5. Load manifest, detect change type, update, save
    ChangeDetector.LoadManifest(ProjectDir);

    bool bStructural = ChangeDetector.IsStructuralChange(Blueprint->ClassName, ManifestEntry);
    Result.bIsStructuralChange = bStructural;

    ChangeDetector.UpdateEntry(Blueprint->ClassName, ManifestEntry);
    ChangeDetector.SaveManifest(ProjectDir);

    return Result;
}



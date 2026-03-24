using UnrealBuildTool;

public class CodeForgeEditor : ModuleRules
{
    public CodeForgeEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "CodeForgeRuntime"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "UnrealEd",
            "GraphEditor",
            "PropertyEditor",
            "InputCore",
            "AssetTools",
            "ToolMenus",
            "Projects",
            "Json",
            "JsonUtilities",
            "DeveloperSettings",
            "AssetRegistry",
            "ContentBrowser"
        });
    }
}


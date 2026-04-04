// Copyright (c) 2026 GregOrigin. All Rights Reserved.

using UnrealBuildTool;

public class CodeForgeRuntime : ModuleRules
{
    public CodeForgeRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine"
        });

        // Automation test framework â€” available in editor and test builds only
        if (Target.bBuildEditor || Target.Configuration == UnrealTargetConfiguration.Test
            || Target.Configuration == UnrealTargetConfiguration.Development)
        {
            PrivateDependencyModuleNames.Add("AutomationController");
        }
    }
}



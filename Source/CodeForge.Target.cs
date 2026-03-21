using UnrealBuildTool;

public class CodeForgeTarget : TargetRules
{
	public CodeForgeTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("CodeForge");
	}
}

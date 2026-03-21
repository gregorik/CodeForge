using UnrealBuildTool;

public class CodeForgeEditorTarget : TargetRules
{
	public CodeForgeEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("CodeForge");
	}
}

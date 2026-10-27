using UnrealBuildTool;
public class SteamModEditor : ModuleRules
{
    public SteamModEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[] {
            "Core", "CoreUObject", "Engine", "Slate", "SlateCore", "ToolMenus",
            "PropertyEditor", "AssetTools", "UnrealEd", "Json", "Projects", "PakFile", "SteamMod", "DesktopPlatform", "NavigationSystem"
        });
    }
}

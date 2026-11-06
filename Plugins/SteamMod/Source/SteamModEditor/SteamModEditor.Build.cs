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

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            AddEngineThirdPartyPrivateStaticDependencies(Target, "Steamworks");
            PublicDelayLoadDLLs.Add("steam_api64.dll");
            RuntimeDependencies.Add("$(TargetOutputDir)/steam_api64.dll", "$(EngineDir)/Binaries/ThirdParty/Steamworks/Steamv161/Win64/steam_api64.dll");
            PublicDefinitions.Add("STEAMMOD_EDITOR_WITH_STEAMWORKS=1");
        }
        else
        {
            PublicDefinitions.Add("STEAMMOD_EDITOR_WITH_STEAMWORKS=0");
        }
    }
}

using UnrealBuildTool;

public class CodexUnrealBridge : ModuleRules
{
    public CodexUnrealBridge(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "EditorSubsystem" });
        PrivateDependencyModuleNames.AddRange(new[]
        {
            "UnrealEd", "Kismet", "BlueprintGraph", "BlueprintEditorLibrary",
            "Json", "JsonUtilities", "AssetRegistry"
        });
    }
}

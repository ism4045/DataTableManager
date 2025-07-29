using System.IO;
using UnrealBuildTool;
 
public class DataTableModule : ModuleRules
{
	public DataTableModule(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDefinitions.Add("WITH_MYTHIRDPARTYLIBRARY=1");

        string ThirdPartyPath = Path.Combine(ModuleDirectory, "ThirdParty");

        PublicIncludePaths.AddRange(
            new string[]
            {
                Path.Combine(ThirdPartyPath, "include", "OpenXLSX"),
                Path.Combine(ThirdPartyPath, "include", "OpenXLSX", "headers"),
                Path.Combine(ThirdPartyPath, "include", "OpenXLSX", "external", "pugixml"),
            });

        if (Target.Configuration == UnrealTargetConfiguration.Debug)
        {
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "lib", "OpenXLSX", "OpenXLSXd.lib"));
        }
        else
        {
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "lib", "OpenXLSX", "OpenXLSX.lib"));
        }

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UnrealEd", "Slate", "SlateCore", "EditorStyle", "ToolMenus", "Projects", "UMG", "AssetTools", "AssetRegistry" });

        PublicIncludePaths.AddRange(new string[] {"DataTableModule/Module/Public", "DataTableModule/Widget/Public", "DataTableModule/Utility/Public" });
        PrivateIncludePaths.AddRange(new string[] { "DataTableModule/Module/Private", "DataTableModule/Widget/Private", "DataTableModule/Utility/Private" });
	}
}
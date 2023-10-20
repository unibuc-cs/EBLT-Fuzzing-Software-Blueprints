// Some copyright should be here...

using UnrealBuildTool;

public class EBLT : ModuleRules
{
	public EBLT(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "AIModule", "CinematicCamera"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Json",
			"AIModule",
			"CinematicCamera",
            "Projects",
            "NavigationSystem"
        });
	}
}

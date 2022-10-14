// Some copyright should be here...

using UnrealBuildTool;

public class EBLT : ModuleRules
{
	public EBLT(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "AIModule"
        });

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Json",
			"AIModule"
		});
	}
}

// Some copyright should be here...

using UnrealBuildTool;
public class UE4VNC : ModuleRules
{
    public UE4VNC(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        AddEngineThirdPartyPrivateStaticDependencies(Target, "zlib");

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Sockets",
                "Networking",
                "ImageWrapper",
		        // ... add other public dependencies that you statically link with here ...
	        }
        );
    }
}
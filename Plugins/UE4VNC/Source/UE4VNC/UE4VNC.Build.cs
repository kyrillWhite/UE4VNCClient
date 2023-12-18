// Some copyright should be here...

using System.IO;
using UnrealBuildTool;
public class UE4VNC : ModuleRules
{
    public UE4VNC(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        AddEngineThirdPartyPrivateStaticDependencies(Target, "zlib");
        PublicDefinitions.Add("WIN32_LEAN_AND_MEAN");

        string FfmpegDirectory = "../../ThirdParty/ffmpeg/lib";
        string[] FfmpegLibNames = {
            "avutil",
            "swresample",
            "avcodec",
            "swscale",
            "avformat",
            "postproc",
            "avfilter",
            "avdevice",
        };
        foreach (var libName in FfmpegLibNames)
        {
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, FfmpegDirectory, libName + ".lib"));
        }

        string srtDirectory = "../../ThirdParty/srt/lib";
        string[] srtLibNames = {
            //"srt",
            "srt_static",
        };
        foreach (var libName in srtLibNames)
        {
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, srtDirectory, libName + ".lib"));
        }
        //string[] dllNames = {
        //    "avutil-58",
        //    "swresample-4",
        //    "avcodec-60",
        //    "swscale-7",
        //    "avformat-60",
        //    "postproc-57",
        //    "avfilter-9",
        //    "avdevice-60",
        //};
        //foreach (string dllName in dllNames)
        //{
        //    PublicDelayLoadDLLs.Add(dllName + ".dll");
        //}

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
                "AVEncoder",
                "RHI",
                "RenderCore",
                "Projects",
                "ImageWriteQueue"
		        // ... add other public dependencies that you statically link with here ...
	        }
        );
    }
}
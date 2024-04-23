// Copyright Epic Games, Inc. All Rights Reserved.

#include "UE4VNC.h"
#include "Interfaces/IPluginManager.h"
#include "HAL/FileManager.h"

#define LOCTEXT_NAMESPACE "FUE4VNCModule"

void FUE4VNCModule::StartupModule()
{

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	//FString pluginPath = IPluginManager::Get().FindPlugin("UE4VNC")->GetBaseDir();
	//UE_LOG(LogTemp, Error, TEXT("AIMHello %s"), *pluginPath);
	//TArray<FString> names = {
	//	"avutil-58.dll",
	//	"swresample-4.dll",
	//	"avcodec-60.dll",
	//	"swscale-7.dll",
	//	"avformat-60.dll",
	//	"postproc-57.dll",
	//	"avfilter-9.dll",
	//	"avdevice-60.dll",
	//};
	//FString dllPath = pluginPath + "/ThirdParty/ffmpeg/";
	//FString absPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*dllPath);
	////FPlatformProcess::AddDllDirectory(*absPath);

	//for (auto name : names) {
	//	//FString dllFolderPath = pluginPath + "/ThirdParty/ffmpeg/" + name;
	//	//UE_LOG(LogTemp, Error, TEXT("AIMHello %s"), *dllFolderPath);
	//	//DLLHandles.Add(FPlatformProcess::GetDllHandle(*dllFolderPath));
	//	FString dllP = absPath + name;
	//	DLLHandles.Add(FPlatformProcess::GetDllHandle(*dllP));
	//}
}

void FUE4VNCModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	/*for (auto DLLHandle : DLLHandles) {
		FPlatformProcess::FreeDllHandle(DLLHandle);
	}*/
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUE4VNCModule, UE4VNC)
// Copyright Alex Quevillon. All Rights Reserved.

#include "UtuPlugin/Scripts/Public/UtuPluginPaths.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformMisc.h"
#include "Kismet/KismetSystemLibrary.h"

// Utilities
bool UtuPluginPaths::isConstructed = false;
// Windows
const FString UtuPluginPaths::slash = "/";
const FString UtuPluginPaths::backslash = "\\";
// Plugin
const FString UtuPluginPaths::default_pluginFile_Full_Config = "{%AppData%}/AlexQuevillon/UtuPlugin/Unreal_Config.json";
FString UtuPluginPaths::pluginFile_Full_Config;
const FString UtuPluginPaths::default_pluginFolder_Full_Exports = "{%AppData%}/AlexQuevillon/UtuPlugin/Exports";
FString UtuPluginPaths::pluginFolder_Full_Exports;

void UtuPluginPaths::ConstructUtuPluginPaths() {
	// Utilities
	UtuPluginPaths::isConstructed = true;
	// Plugin
#if PLATFORM_WINDOWS
	UtuPluginPaths::pluginFile_Full_Config = UtuPluginPaths::default_pluginFile_Full_Config.Replace(TEXT("{%AppData%}"), *FPlatformMisc::GetEnvironmentVariable(TEXT("AppData")));
	UtuPluginPaths::pluginFolder_Full_Exports = UtuPluginPaths::default_pluginFolder_Full_Exports.Replace(TEXT("{%AppData%}"), *FPlatformMisc::GetEnvironmentVariable(TEXT("AppData")));
#else
	UtuPluginPaths::pluginFile_Full_Config = UtuPluginPaths::default_pluginFile_Full_Config.Replace(TEXT("{%AppData%}"), *FString("/Users/" + UKismetSystemLibrary::GetPlatformUserName() + "/Documents/"));
    UtuPluginPaths::pluginFolder_Full_Exports = UtuPluginPaths::default_pluginFolder_Full_Exports.Replace(TEXT("{%AppData%}"),  *FString("/Users/" + UKismetSystemLibrary::GetPlatformUserName() + "/Documents/"));
#endif
}

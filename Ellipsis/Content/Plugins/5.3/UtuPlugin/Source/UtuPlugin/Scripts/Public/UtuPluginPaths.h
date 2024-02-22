// Copyright Alex Quevillon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UTUPLUGIN_API UtuPluginPaths {
public:
	// Utilities
	static bool isConstructed;
	// Windows
	static const FString slash;
	static const FString backslash;
	// Plugin
	static const FString default_pluginFile_Full_Config;
	static FString pluginFile_Full_Config;
	static const FString default_pluginFolder_Full_Exports;
	static FString pluginFolder_Full_Exports;

public:
	static void ConstructUtuPluginPaths();
};
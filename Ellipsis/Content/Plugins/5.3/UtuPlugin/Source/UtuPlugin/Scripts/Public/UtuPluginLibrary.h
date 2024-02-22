// Copyright Alex Quevillon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "UtuPluginLibrary.generated.h"

class UWidgetBlueprint;
class SDockTab;
class SWidget;

UCLASS()
class UTUPLUGIN_API UUtuPluginLibrary : public UBlueprintFunctionLibrary {
	GENERATED_BODY()
public:
	static void CloseAllSpawnedTabs();
	static TArray<TSharedRef<SDockTab>> SpawnedTabs;
	static void OpenWidget(UWidgetBlueprint* InBlueprint, FText InDisplayName);
	static bool DoesAssetExists(FString InAssetRelativeFilename);
	static UObject* TryGetAsset(FString InAssetRelativeFilename);
	static bool DeleteAsset(UObject* InAsset);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static bool DoesWindowsFileExists(FString InFileFullname);
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static bool DoesWindowsFolderExists(FString InFolderFullname);
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static void CopyWindowsFolder(FString InFolderFullnameSource, FString InFolderFullnameDestination);
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static void DeleteWindowsFolder(FString InFolderFullname);
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static void DeleteWindowsFile(FString InFileFullname);
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static void CopyWindowsFile(FString InFileFullnameSource, FString InFileFullnameDestination);
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static FString GetWindowsFolderFullnameFromDialog(FString InStartingPath, FString InWindowTitle);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static void CopyTextToClipboard(FString InText);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static void GetPathsNeededToInstallPluginInUnityProject(FString InUnityProjectAssetsFolderFullname, FString& OutUtuPluginFolder, FString& OutSource, FString& OutDestination, FString& OutVersion, FString& OutVersionFile, FString& OutDuplicatedDestination, FString InSelectedVersion = "2018+");
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static void InstallPluginInUnityProject(FString InUnityProjectAssetsFolderFullname, FString InSelectedVersion = "2018+");
};

// Copyright Alex Quevillon. All Rights Reserved.

#include "UtuPlugin/Scripts/Public/UtuPluginLibrary.h"
#include "UtuPlugin/Scripts/Public/UtuPluginLog.h"

#include "Modules/ModuleManager.h"
#include "LevelEditor.h"
#include "Editor/UMGEditor/Public/WidgetBlueprint.h"
#include "Editor/Blutility/Classes/EditorUtilityWidget.h"
#include "Editor/Blutility/Public/IBlutilityModule.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformMisc.h"
#include "Editor/UnrealEd/Public/ObjectTools.h"
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/AssetRegistryInterface.h"
#else
#include "Runtime/AssetRegistry/Public/AssetRegistryModule.h"
#include "Runtime/AssetRegistry/Public/IAssetRegistry.h"
#endif
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Runtime/Slate/Public/Framework/Application/SlateApplication.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Slate/Public/Widgets/Docking/SDockTab.h"
#include "Interfaces/IPluginManager.h" 

#define LOCTEXT_NAMESPACE "UtuPluginLibrary"

void UUtuPluginLibrary::OpenWidget(UWidgetBlueprint* InBlueprint, FText InDisplayName) {
	if (InBlueprint != nullptr && GEditor != nullptr) {
		// Register Tab
		FName RegistrationName = FName(*InBlueprint->GetPathName());
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(RegistrationName);
		//if (!FGlobalTabmanager::Get()->CanSpawnTab(RegistrationName)) {
			auto SpawnTabLambda = [](const FSpawnTabArgs&) -> TSharedRef<SDockTab> { return SNew(SDockTab).TabRole(ETabRole::NomadTab); };
			FGlobalTabmanager::Get()->RegisterNomadTabSpawner(RegistrationName, FOnSpawnTab::CreateLambda(SpawnTabLambda)).SetDisplayName(InDisplayName);
		//}

		// SetTab Content
		TSharedRef<SWidget> TabWidget = SNullWidget::NullWidget;
		UClass* BlueprintClass = InBlueprint->GeneratedClass;
		TSubclassOf<UUserWidget> WidgetClass = BlueprintClass;
		UUserWidget* CreatedUMGWidget = NewObject<UUserWidget>(GEditor, WidgetClass, RegistrationName, RF_Transactional);
		if (CreatedUMGWidget != nullptr) {
			TSharedRef<SWidget> CreatedSlateWidget = CreatedUMGWidget->TakeWidget();
			TabWidget = SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Fill)
				[
					CreatedSlateWidget
				];
		}
		CreatedUMGWidget->Initialize();
		// Spawn Tab
#if ENGINE_MAJOR_VERSION >= 5
		TSharedRef<SDockTab> NewDockTab = FGlobalTabmanager::Get()->TryInvokeTab(RegistrationName).ToSharedRef();
#else
		TSharedRef<SDockTab> NewDockTab = FGlobalTabmanager::Get()->InvokeTab(RegistrationName);
#endif
		NewDockTab->SetContent(TabWidget);
		FGlobalTabmanager::Get()->DrawAttention(NewDockTab);
		// To close them later
		UUtuPluginLibrary::SpawnedTabs.AddUnique(NewDockTab);
		GEditor->OnEditorClose().AddStatic(UUtuPluginLibrary::CloseAllSpawnedTabs);
	}
	//FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	//TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();
	//if (!LevelEditorTabManager->CanSpawnTab(RegistrationName)) {
	//	auto SpawnTabLambda = [](const FSpawnTabArgs&) -> TSharedRef<SDockTab> { return SNew(SDockTab).TabRole(ETabRole::NomadTab); };
	//	LevelEditorTabManager->RegisterTabSpawner(RegistrationName, FOnSpawnTab::CreateLambda(SpawnTabLambda)).SetDisplayName(InDisplayName);
	//}
	//TSharedRef<SDockTab> NewDockTab = LevelEditorTabManager->InvokeTab(RegistrationName);
}

// List of spawned tabs that you want to close
TArray<TSharedRef<SDockTab>> UUtuPluginLibrary::SpawnedTabs;
// Static function that closes them
void UUtuPluginLibrary::CloseAllSpawnedTabs() {
	for (TSharedRef<SDockTab> Tab : UUtuPluginLibrary::SpawnedTabs) {
		Tab->RequestCloseTab();
	}
	UUtuPluginLibrary::SpawnedTabs.Empty(); // Not clearing this make the engine crash
}


bool UUtuPluginLibrary::DoesWindowsFileExists(FString InFileFullname) {
	return FPaths::FileExists(InFileFullname);
}

bool UUtuPluginLibrary::DoesWindowsFolderExists(FString InFolderFullname) {
	return FPaths::DirectoryExists(InFolderFullname);
}

void UUtuPluginLibrary::CopyWindowsFolder(FString InFolderFullnameSource, FString InFolderFullnameDestination) {
	if (UUtuPluginLibrary::DoesWindowsFolderExists(InFolderFullnameDestination)) {
		UUtuPluginLibrary::DeleteWindowsFolder(InFolderFullnameDestination);
	}
	IFileManager::Get().MakeDirectory(*InFolderFullnameDestination, true);
	//IFileManager::Get().CopyDirectoryTree(*InFolderFullnameDestination, *InFolderFullnameSource);
	IPlatformFile::GetPlatformPhysical().CopyDirectoryTree(*InFolderFullnameDestination, *InFolderFullnameSource, true);
	//FPlatformFileManager::Get().GetPlatformFile().CopyDirectoryTree(*InFolderFullnameDestination, *InFolderFullnameSource, true);
}

void UUtuPluginLibrary::DeleteWindowsFolder(FString InFolderFullname) {
	IFileManager::Get().DeleteDirectory(*InFolderFullname, false, true);
}

void UUtuPluginLibrary::DeleteWindowsFile(FString InFileFullname) {
	IFileManager::Get().Delete(*InFileFullname, false, false, true);
}

void UUtuPluginLibrary::CopyWindowsFile(FString InFileFullnameSource, FString InFileFullnameDestination) {
	if (UUtuPluginLibrary::DoesWindowsFileExists(InFileFullnameDestination)) {
		UUtuPluginLibrary::DeleteWindowsFile(InFileFullnameDestination);
	}
	IFileManager::Get().Copy(*InFileFullnameDestination, *InFileFullnameSource);
}

FString UUtuPluginLibrary::GetWindowsFolderFullnameFromDialog(FString InStartingPath, FString InWindowTitle) {
	FString OpenFoldername = "";
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform) {
		if (!DesktopPlatform->OpenDirectoryDialog(FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr), InWindowTitle, InStartingPath, OpenFoldername)) {
			return "";
		}
	}
	return FPaths::ConvertRelativePathToFull(OpenFoldername);
}

void UUtuPluginLibrary::CopyTextToClipboard(FString Text) {
	FGenericPlatformMisc::ClipboardCopy(*Text);
}

void UUtuPluginLibrary::GetPathsNeededToInstallPluginInUnityProject(FString InUnityProjectAssetsFolderFullname, FString& OutUtuPluginFolder, FString& OutSource, FString& OutDestination, FString& OutVersion, FString& OutVersionFile, FString& OutDuplicatedDestination, FString InSelectedVersion) {
	OutUtuPluginFolder = IPluginManager::Get().FindPlugin("UtuPlugin")->GetBaseDir();
	OutSource = OutUtuPluginFolder / "Resources/UnityPlugin~" / InSelectedVersion / "UtuPlugin";
	OutDestination = InUnityProjectAssetsFolderFullname.Replace(TEXT("\\"), TEXT("/")) / "AlexQuevillon/UtuPlugin";
	// Find UE4 Version
	OutVersion = "4." + FString::FromInt(ENGINE_MINOR_VERSION);
	OutVersionFile = OutUtuPluginFolder / "Resources" / OutVersion;
	if (!UUtuPluginLibrary::DoesWindowsFileExists(OutVersionFile)) {
		FFileHelper::SaveStringToFile(TEXT(""), *OutVersionFile);
	}
	OutDuplicatedDestination = OutDestination + "/UnrealPlugin~/" + OutVersion + "/UtuPlugin";
}

void UUtuPluginLibrary::InstallPluginInUnityProject(FString InUnityProjectAssetsFolderFullname, FString InSelectedVersion) {
	FString UtuPluginFolder, Source, Destination, Version, VersionFile, DuplicatedDestination;
	GetPathsNeededToInstallPluginInUnityProject(InUnityProjectAssetsFolderFullname, UtuPluginFolder, Source, Destination, Version, VersionFile, DuplicatedDestination, InSelectedVersion);
	UTU_LOG_L("Installing Plugins into Unity Project: " + InUnityProjectAssetsFolderFullname);
	UTU_LOG_L("    Source: " + Source);
	UTU_LOG_L("    Destination: " + Destination);
	UTU_LOG_L("    UE4 Version: " + Version);
	UTU_LOG_L("    UE4 Destination in Unity Plugin: " + DuplicatedDestination);
	// Copy in Unity Project

	if (!UUtuPluginLibrary::DoesWindowsFolderExists(Source) || !UUtuPluginLibrary::DoesWindowsFileExists(VersionFile)) {
		UTU_LOG_E("    Either the Source directory or the version file doesn't exist! Cannot install the plugin in Unity!");
		return;
	}
	UUtuPluginLibrary::CopyWindowsFolder(Source, Destination);
	UUtuPluginLibrary::CopyWindowsFolder(UtuPluginFolder, DuplicatedDestination);
	// Delete the Unity plugin from the Unreal plugin
	FString Destination_UnityPluginsFolder = DuplicatedDestination / "Resources/UnityPlugin~";
	UUtuPluginLibrary::DeleteWindowsFolder(Destination_UnityPluginsFolder);
}

bool UUtuPluginLibrary::DoesAssetExists(FString InAssetRelativeFilename) {
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(AssetRegistryConstants::ModuleName);
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(*InAssetRelativeFilename);
	return AssetData.IsValid();
}

UObject* UUtuPluginLibrary::TryGetAsset(FString InAssetRelativeFilename) {
	return StaticLoadObject(UObject::StaticClass(), nullptr, *InAssetRelativeFilename);
	if (DoesAssetExists(InAssetRelativeFilename)) {
		return StaticLoadObject(UObject::StaticClass(), nullptr, *InAssetRelativeFilename);
	}
	return nullptr;
}

bool UUtuPluginLibrary::DeleteAsset(UObject* InAsset) {
	if (GEditor != nullptr && GEditor->GetPIEWorldContext() == nullptr) {
		return ObjectTools::DeleteObjects({ InAsset }, /*bShowConfirmation=*/false) > 0;
	}
	return false;
}


#undef LOCTEXT_NAMESPACE

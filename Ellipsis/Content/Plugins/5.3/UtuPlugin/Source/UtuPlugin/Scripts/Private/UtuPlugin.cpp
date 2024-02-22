// Copyright Alex Quevillon. All Rights Reserved.

#include "UtuPlugin/Scripts/Public/UtuPlugin.h"
#include "UtuPlugin/Scripts/Public/UtuPluginLog.h"
#include "UtuPlugin/Scripts/Public/UtuPluginPaths.h"
#include "UtuPlugin/Scripts/Public/UtuPluginLibrary.h"

#include "Runtime/Core/Public/Misc/DateTime.h"
#include "Editor/UnrealEd/Public/FileHelpers.h"
#include "Runtime/Slate/Public/Framework/Application/SlateApplication.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Interfaces/IPluginManager.h"

void FUtuPluginCurrentImport::Import(FUtuPluginJson Json, TArray<EUtuAssetType> AssetTypes, bool executeFullImportOnSameFrame, bool SaveAllAfterProcess, bool DeleteInvalidAssets) {
	BeginImport(Json, AssetTypes, SaveAllAfterProcess, DeleteInvalidAssets);
	if (executeFullImportOnSameFrame) {
		while (ContinueImport(executeFullImportOnSameFrame) != true) {
			// ContinueImport
		}
	}
}

void FUtuPluginCurrentImport::BeginImport(FUtuPluginJson Json, TArray<EUtuAssetType> AssetTypes, bool SaveAllAfterProcess, bool DeleteInvalidAssets) {
	TArray<EUtuAssetType> assetTypesOrder = { EUtuAssetType::Texture, EUtuAssetType::Material, EUtuAssetType::Mesh, EUtuAssetType::PrefabFirstPass, EUtuAssetType::PrefabSecondPass, EUtuAssetType::Scene };
	assetTypesToProcess.Empty();
	for (EUtuAssetType AssetType : assetTypesOrder) {
		if (AssetTypes.Contains(AssetType)) {
			assetTypesToProcess.Add(AssetType); // Order is important
		}
	}
	bSaveAllAfterProcess = SaveAllAfterProcess;
	bDeleteInvalidAssets = DeleteInvalidAssets;
	countAssetTypesToProcess = 1;
	amountAssetTypesToProcess = assetTypesToProcess.Num();
	percentAssetTypesToProcess = (float)countAssetTypesToProcess / (float)amountAssetTypesToProcess;
	json = Json;
	timestamp = FDateTime::UtcNow().ToString().Replace(TEXT("-"), TEXT("_")).Replace(TEXT("."), TEXT(""));
	UUtuPluginLog::InitializeNewLog(json.json_info.export_timestamp);
	// Log
	UTU_LOG_CLEAR();
	UTU_LOG_SEPARATOR_LINE();
	UTU_LOG_L("Beginning New Import...");
	UTU_LOG_L("    Time: " + FDateTime::UtcNow().ToString());
	UTU_LOG_L("    Utu Version: " + UUtuPlugin::GetUtuPluginVersion());
	UTU_LOG_L("    Json File Fullname: " + json.json_info.json_file_fullname);
	UTU_LOG_L("        Export Name: " + json.json_info.export_name);
	UTU_LOG_L("        Export Time: " + json.json_info.export_datetime);
	UTU_LOG_L("        Export Utu Version: " + json.json_info.utu_plugin_version);
	UTU_LOG_L("        Export Scene Quantity: " + FString::FromInt(json.json_info.scenes.Num()));
	UTU_LOG_L("        Export Prefab Quantity: " + FString::FromInt(json.json_info.prefabs.Num()));
	UTU_LOG_L("        Export Mesh Quantity: " + FString::FromInt(json.json_info.meshes.Num()));
	UTU_LOG_L("        Export Material Quantity: " + FString::FromInt(json.json_info.materials.Num()));
	UTU_LOG_L("        Export Texture Quantity: " + FString::FromInt(json.json_info.textures.Num()));
	UTU_LOG_L("    Save All after process: " + FString(bSaveAllAfterProcess ? "true" : "false"));
	UTU_LOG_L("    Delete Invalid Assets: " + FString(bDeleteInvalidAssets ? "true" : "false"));
	UTU_LOG_L("    Asset Types to process: ");
	for (EUtuAssetType AssetType : assetTypesToProcess) {
		UTU_LOG_L("        " + AssetTypeToString(AssetType));
	}
	UTU_LOG_SEPARATOR_LINE();
}

bool FUtuPluginCurrentImport::ContinueImport(bool executeFullImportOnSameFrame) {
	if (!currentAssetTypeProcessor.bIsValid && assetTypesToProcess.Num() == 0) {
		//UTU_LOG_E("FUtuPluginCurrentImport::ContinueImport() Was called even though the list is already empty. This should never happen!");
		CompleteImport();
		return true;
	}
	if (!currentAssetTypeProcessor.bIsValid && assetTypesToProcess.Num() > 0) {
		currentAssetTypeProcessor = FUtuPluginAssetTypeProcessor();
		currentAssetTypeProcessor.bIsValid = true;
		currentAssetTypeProcessor.ImportSettings = ImportSettings;
		namEUtuAssetTypesToProcess = AssetTypeToString(assetTypesToProcess[0]);
		currentAssetTypeProcessor.Import(json, assetTypesToProcess[0], executeFullImportOnSameFrame, bDeleteInvalidAssets);
		assetTypesToProcess.RemoveAt(0);
		UTU_LOG_SEPARATOR_LINE();
		UTU_LOG_L("Starting to import assets of type: " + namEUtuAssetTypesToProcess + "...");
		UTU_LOG_L("    Time: " + FDateTime::UtcNow().ToString());
		UTU_LOG_L("    Quantity: " + FString::FromInt(currentAssetTypeProcessor.GetAssetsNum()));
		UTU_LOG_SEMI_SEPARATOR_LINE();
	}
	if (executeFullImportOnSameFrame) {
		currentAssetTypeProcessor.bIsValid = false;
		countAssetTypesToProcess = amountAssetTypesToProcess;
		percentAssetTypesToProcess = (float)countAssetTypesToProcess / (float)FMath::Max(amountAssetTypesToProcess, 1);
	}
	else {
		if (currentAssetTypeProcessor.ContinueImport()) {
			currentAssetTypeProcessor.bIsValid = false;
			countAssetTypesToProcess++;
			percentAssetTypesToProcess = (float)countAssetTypesToProcess / (float)FMath::Max(amountAssetTypesToProcess, 1);
		}
	}
	if (!currentAssetTypeProcessor.bIsValid && assetTypesToProcess.Num() == 0) {
		CompleteImport();
		return true;
	}
	return false;
}

void FUtuPluginCurrentImport::CompleteImport() {
	UTU_LOG_SEPARATOR_LINE();
	UTU_LOG_L("Completing Import...");
	UTU_LOG_L("    Time: " + FDateTime::UtcNow().ToString());
	UTU_LOG_L("    Save All after process: " + FString(bSaveAllAfterProcess ? "true" : "false"));
	UTU_LOG_L("    Delete Invalid Assets: " + FString(bDeleteInvalidAssets ? "true" : "false"));
	if (bSaveAllAfterProcess) {
		UTU_LOG_L("        Prompt user for save all...");
		TArray<UPackage*> Packages;
		FEditorFileUtils::GetDirtyContentPackages(Packages);
		FEditorFileUtils::GetDirtyWorldPackages(Packages);
		FEditorFileUtils::PromptForCheckoutAndSave(Packages, false, true);
		//FEditorFileUtils::SaveDirtyPackages(false, true, true);
	}
	UTU_LOG_SEPARATOR_LINE();
	UTU_LOG_L("Import Completed!");
	UTU_LOG_L("    Time: " + FDateTime::UtcNow().ToString());
	EUtuLog LogState;
	int WarningCount;
	int ErrorCount;
	UUtuPluginLog::GetLogState(LogState, WarningCount, ErrorCount);
	UTU_LOG("    Warning Count: " + FString::FromInt(WarningCount), WarningCount > 0 ? EUtuLog::Warning : EUtuLog::Log);
	UTU_LOG("    Error   Count: " + FString::FromInt(ErrorCount), ErrorCount > 0 ? EUtuLog::Error : EUtuLog::Log);
	UTU_LOG_SEPARATOR_LINE();
	UUtuPluginLog::PrintIntoLogFile("", true);
}

FString FUtuPluginCurrentImport::AssetTypeToString(EUtuAssetType AssetType) {
	switch (AssetType) {
	case EUtuAssetType::Scene:
		return "Scenes";
		break;
	case EUtuAssetType::Mesh:
		return "Meshes";
		break;
	case EUtuAssetType::Material:
		return "Materials";
		break;
	case EUtuAssetType::Texture:
		return "Textures";
		break;
	case EUtuAssetType::PrefabFirstPass:
		return "Prefabs: First Pass";
		break;
	case EUtuAssetType::PrefabSecondPass:
		return "Prefabs: Second Pass";
		break;
	}
	return "";
}

FUtuPluginCurrentImport UUtuPlugin::currentImportJob;
FUtuPluginImportSettings UUtuPlugin::currentImportSettings;

void UUtuPlugin::Import(FUtuPluginJson Json, TArray<EUtuAssetType> AssetTypes, bool executeFullImportOnSameFrame, bool SaveAllAfterProcess, bool DeleteInvalidAssets) {
	static FTick TickInstance;
	currentImportJob = FUtuPluginCurrentImport();
	currentImportJob.bIsValid = true;
	currentImportJob.ImportSettings = UUtuPlugin::currentImportSettings;
	currentImportJob.Import(Json, AssetTypes, executeFullImportOnSameFrame, SaveAllAfterProcess, DeleteInvalidAssets);
	if (executeFullImportOnSameFrame) {
		currentImportJob.bIsValid = false;
	}
}

FUtuPluginCurrentImport UUtuPlugin::GetCurrentImportState() {
	return UUtuPlugin::currentImportJob;
}

void UUtuPlugin::SetImportSettings(FUtuPluginImportSettings ImportSettings)
{
	UUtuPlugin::currentImportSettings = ImportSettings;
}

FString UUtuPlugin::GetUtuPluginVersion()
{
	FString TextString;
	FString FilePath = IPluginManager::Get().FindPlugin(TEXT("UtuPlugin"))->GetBaseDir() + "/Resources/UtuPluginVersion.txt";
	FFileHelper::LoadFileToString(TextString, *FilePath);
	return TextString;
}

FString UUtuPlugin::GetUtuPluginInfo()
{
	FString TextString;
	FString FilePath = IPluginManager::Get().FindPlugin(TEXT("UtuPlugin"))->GetBaseDir() + "/Resources/UtuPluginInfo.txt";
	FFileHelper::LoadFileToString(TextString, *FilePath);
	return TextString;
}

FUtuPluginCurrentImport UUtuPlugin::ContinueCurrentImport() {
	if (currentImportJob.bIsValid) {
		if (currentImportJob.ContinueImport(false)) {
			currentImportJob.bIsValid = false;
		}
	}
	return currentImportJob;
}

void UUtuPlugin::CancelImport() {
	currentImportJob.bIsValid = false;
	UUtuPluginLog::PrintIntoLogFile("\n\n\n\n\n\nImport Cancelled By User!", true);
}

void FTick::Tick(float DeltaTime) {
	static int TickCount = 0; // To give enough time to the user to cancel
	TickCount++;
	if (TickCount == 10) {
		UUtuPlugin::ContinueCurrentImport();
		TickCount = 0;
	}
}

TStatId FTick::GetStatId() const {
	RETURN_QUICK_DECLARE_CYCLE_STAT(FTick, STATGROUP_Tickables);
}

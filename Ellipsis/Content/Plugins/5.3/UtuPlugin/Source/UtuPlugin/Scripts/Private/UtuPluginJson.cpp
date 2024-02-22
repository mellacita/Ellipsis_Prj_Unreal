// Copyright Alex Quevillon. All Rights Reserved.

#include "UtuPlugin/Scripts/Public/UtuPluginJson.h"
#include "UtuPlugin/Scripts/Public/UtuPluginPaths.h"
#include "UtuPlugin/Scripts/Public/UtuPluginLibrary.h"

#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Json/Public/Serialization/JsonReader.h"
#include "Runtime/Json/Public/Dom/JsonObject.h"
#include "Runtime/Json/Public/Serialization/JsonSerializer.h"
#include "Runtime/JsonUtilities/Public/JsonObjectConverter.h"

FUtuPluginJson UUtuPluginJsonUtilities::ReadExportJsonFromFile(FString JsonFile) {
	FUtuPluginJson Json;
	FString JsonString;
	if (FFileHelper::LoadFileToString(JsonString, *JsonFile)) {
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
		TSharedPtr<FJsonObject> JsonObject;
		if (FJsonSerializer::Deserialize(JsonReader, JsonObject)) {
			FJsonObjectConverter::JsonObjectToUStruct<FUtuPluginJson>(JsonObject.ToSharedRef(), &Json);
		}
	}
	return Json;
}

TArray<FString> UUtuPluginJsonUtilities::GetAvailableExportJsons() {
	TArray<FString> Ret;
	if (UUtuPluginLibrary::DoesWindowsFolderExists(UtuPluginPaths::pluginFolder_Full_Exports)) {
		TArray<FString> Folders;
		IFileManager::Get().FindFiles(Folders, *(UtuPluginPaths::pluginFolder_Full_Exports + "/*"), false, true);
		Folders.Sort();
		for (int X = Folders.Num() - 1; X >= 0; X--) {
			Ret.Add(UtuPluginPaths::pluginFolder_Full_Exports + UtuPluginPaths::slash + Folders[X] + UtuPluginPaths::slash + "UtuPlugin.json");
		}
	}
	return Ret;
}

FUtuPluginJsonInfo UUtuPluginJsonUtilities::ReadExportJsonInfoFromFile(FString JsonFile) {
	FUtuPluginJsonInfo Json;
	FString JsonString;
	if (FFileHelper::LoadFileToString(JsonString, *JsonFile)) {
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
		TSharedPtr<FJsonObject> JsonObject;
		if (FJsonSerializer::Deserialize(JsonReader, JsonObject)) {
			FJsonObjectConverter::JsonObjectToUStruct<FUtuPluginJsonInfo>(JsonObject.ToSharedRef(), &Json);
		}
	}
	return Json;
}

TArray<FString> UUtuPluginJsonUtilities::GetAvailableExportJsonInfos() {
	TArray<FString> Ret;
	if (UUtuPluginLibrary::DoesWindowsFolderExists(UtuPluginPaths::pluginFolder_Full_Exports)) {
		TArray<FString> Folders;
		IFileManager::Get().FindFiles(Folders, *(UtuPluginPaths::pluginFolder_Full_Exports + "/*"), false, true);
		Folders.Sort();
		for (int X = Folders.Num() - 1; X >= 0; X--) {
			FString FileFullname = UtuPluginPaths::pluginFolder_Full_Exports + UtuPluginPaths::slash + Folders[X] + UtuPluginPaths::slash + "UtuPluginInfo.json";
			if (UUtuPluginLibrary::DoesWindowsFileExists(FileFullname)) {
				Ret.Add(FileFullname);
			}
			else {
				UUtuPluginLibrary::DeleteWindowsFolder(UtuPluginPaths::pluginFolder_Full_Exports + UtuPluginPaths::slash + Folders[X]);
			}
		}
	}
	return Ret;
}

void UUtuPluginJsonUtilities::DeleteExportJson(FString ExportJsonFileFullname) {
	FString Directory;
	ExportJsonFileFullname.Split(UtuPluginPaths::slash, &Directory, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	UUtuPluginLibrary::DeleteWindowsFolder(Directory);
}

FUtuPluginConfigJson UUtuPluginJsonUtilities::ReadConfigJsonFromFile() {
	FString JsonFile = UtuPluginPaths::pluginFile_Full_Config;
	FUtuPluginConfigJson Json;
	FString JsonString;
	if (FFileHelper::LoadFileToString(JsonString, *JsonFile)) {
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
		TSharedPtr<FJsonObject> JsonObject;
		if (FJsonSerializer::Deserialize(JsonReader, JsonObject)) {
			FJsonObjectConverter::JsonObjectToUStruct<FUtuPluginConfigJson>(JsonObject.ToSharedRef(), &Json);
		}
	}
	return Json;
}

void UUtuPluginJsonUtilities::WriteConfigJsonToFile(FUtuPluginConfigJson InConfig) {
	TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject<FUtuPluginConfigJson>(InConfig);
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	FString JsonFile = UtuPluginPaths::pluginFile_Full_Config;
	FFileHelper::SaveStringToFile(JsonString, *JsonFile);
}


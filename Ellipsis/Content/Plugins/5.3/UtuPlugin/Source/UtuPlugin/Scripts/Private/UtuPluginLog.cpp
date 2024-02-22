// Copyright Alex Quevillon. All Rights Reserved.

#include "UtuPlugin/Scripts/Public/UtuPluginLog.h"
#include "UtuPlugin/Scripts/Public/UtuPluginPaths.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h" 

DEFINE_LOG_CATEGORY(UTU);

TArray<FUtuLog> UUtuPluginLog::Log;
FString UUtuPluginLog::LogString;
int UUtuPluginLog::ErrorCount;
int UUtuPluginLog::WarningCount;
EUtuLog UUtuPluginLog::LogState;
FString UUtuPluginLog::Timestamp;
StringOutputDevice* UUtuPluginLog::OutputDevice = nullptr;

void UUtuPluginLog::InitializeNewLog(FString InTimestamp) {
	UUtuPluginLog::Timestamp = InTimestamp;
	FString Path = UtuPluginPaths::pluginFolder_Full_Exports + UtuPluginPaths::slash + UUtuPluginLog::Timestamp + UtuPluginPaths::slash + "UnrealImport.log";
	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*Path)) {
		FString BackupTimestamp = FDateTime::UtcNow().ToString().Replace(TEXT("-"), TEXT("_")).Replace(TEXT("."), TEXT(""));
		FPlatformFileManager::Get().GetPlatformFile().MoveFile(*(Path.Replace(TEXT(".log"), *("_Backup_" + BackupTimestamp + ".log"))), *Path);
	}
	// Redirect log
	if (OutputDevice != nullptr)
	{
		if (OutputDevice->bIsRegistered)
		{
			OutputDevice->UnregisterFromLog();
		}
		OutputDevice->RegisterToLog();
	}
}

void UUtuPluginLog::PrintIntoLogFile(FString Message, bool bForceWrite) {
	static FString Messages = "";
	static int MessageCount = 0;
	Messages += Message + "\n";
	MessageCount++;
	if (bForceWrite || MessageCount >= 200) {
		FString Path = UtuPluginPaths::pluginFolder_Full_Exports + UtuPluginPaths::slash + UUtuPluginLog::Timestamp + UtuPluginPaths::slash + "UnrealImport.log";
		FString FileContent = "";
		FFileHelper::LoadFileToString(FileContent, *Path);
		FFileHelper::SaveStringToFile(FileContent + Messages, *Path);
		MessageCount = 0;
		Messages = "";
		if (OutputDevice != nullptr)
		{
			FString UnrealPath = UtuPluginPaths::pluginFolder_Full_Exports + UtuPluginPaths::slash + UUtuPluginLog::Timestamp + UtuPluginPaths::slash + "UnrealLog.log";
			FFileHelper::SaveStringToFile(OutputDevice->MyLog, *UnrealPath);
		}
	}
}

void UUtuPluginLog::OpenDirectoryInWindowsExplorer(FString InPath) {
	FPlatformProcess::ExploreFolder(*InPath);
}


UUtuPluginLog::UUtuPluginLog()
{
	OutputDevice = new StringOutputDevice();

}

UUtuPluginLog::~UUtuPluginLog()
{
	if (OutputDevice != nullptr)
	{
		if (OutputDevice->bIsRegistered)
		{
			OutputDevice->UnregisterFromLog();
		}
		delete OutputDevice;
		OutputDevice = nullptr;
	}
}

TArray<FUtuLog> UUtuPluginLog::GetLog() {
	return Log;
}

FString UUtuPluginLog::GetLogString() {
	return LogString;
}

void UUtuPluginLog::GetLogState(EUtuLog& OutLogState, int& OutWarningCount, int& OutErrorCount) {
	OutLogState = LogState;
	OutWarningCount = WarningCount;
	OutErrorCount = ErrorCount;
}

void UUtuPluginLog::ClearLog() {
	Log.Empty();
	ErrorCount = 0;
	WarningCount = 0;
	LogState = EUtuLog::Log;
}

void UUtuPluginLog::AddToLog(FString Message, EUtuLog LogCategory) {
	FUtuLog NewLog;
	switch (LogCategory) {
	default:
	case EUtuLog::Log:
		LogString += ("\nL    " + Message);
		PrintIntoLogFile("L    " + Message, false);
		UE_LOG(UTU, Log, TEXT("%s"), *Message);
		break;
	case EUtuLog::Warning:
		WarningCount++;
		if (LogState == EUtuLog::Log) {
			LogState = EUtuLog::Warning;
		}
		LogString += ("\nW    " + Message);
		PrintIntoLogFile("W    " + Message, false);
		UE_LOG(UTU, Warning, TEXT("%s"), *Message);
		break;
	case EUtuLog::Error:
		ErrorCount++;
		LogState = EUtuLog::Error;
		LogString += ("\nE    " + Message);
		PrintIntoLogFile("E    " + Message, false);
		UE_LOG(UTU, Error, TEXT("%s"), *Message);
		break;
	}
	NewLog.Message = Message;
	NewLog.LogCategory = LogCategory;
	Log.Add(NewLog);
}

void StringOutputDevice::RegisterToLog()
{
	MyLog = "";
	if (GLog != nullptr)
	{
		bIsRegistered = true;
		GLog->AddOutputDevice(this);
	}
}

void StringOutputDevice::UnregisterFromLog()
{
	if (GLog != nullptr)
	{
		bIsRegistered = false;
		GLog->RemoveOutputDevice(this);
	}
}

void StringOutputDevice::Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FName& Category)
{
	if (Category.ToString() != "LogHAL")
	{
		MyLog += Category.ToString() + " " + Message;
		if (!MyLog.EndsWith("\n"))
		{
			MyLog += "\n";
		}
	}
}

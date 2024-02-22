// Copyright Alex Quevillon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "Misc/OutputDevice.h"
#include "UtuPluginLog.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(UTU, All, All);

UENUM(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
enum class EUtuLog : uint8 {
	Log, Warning, Error
};

USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuLog {
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		FString Message;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		EUtuLog LogCategory;
};

class StringOutputDevice : public FOutputDevice
{
public:
	void RegisterToLog();
	void UnregisterFromLog();

	FString MyLog = "";
	bool bIsRegistered = false;
protected:
	virtual void Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const class FName& Category) override;

};

UCLASS()
class UTUPLUGIN_API UUtuPluginLog : public UBlueprintFunctionLibrary {
	GENERATED_BODY()
public:
	UUtuPluginLog();
	~UUtuPluginLog();

public:
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static TArray<FUtuLog> GetLog();
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static FString GetLogString();
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static void GetLogState(EUtuLog& OutLogState, int& OutWarningCount, int& OutErrorCount);
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static void InitializeNewLog(FString InTimestamp);
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static void ClearLog();
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static void AddToLog(FString Message, EUtuLog LogCategory);
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static void OpenDirectoryInWindowsExplorer(FString Path);

	static void PrintIntoLogFile(FString Message, bool bForceWrite);

	static FString Timestamp;
private:
	static TArray<FUtuLog> Log;
	static FString LogString;
	static int ErrorCount;
	static int WarningCount;
	static EUtuLog LogState;
	static StringOutputDevice* OutputDevice;

};



#define UTU_LOG(Message, LogCategory) {UUtuPluginLog::AddToLog(Message, LogCategory);}
#define UTU_LOG_L(Message) {UUtuPluginLog::AddToLog(Message, EUtuLog::Log);}
#define UTU_LOG_W(Message) {UUtuPluginLog::AddToLog(Message, EUtuLog::Warning);}
#define UTU_LOG_E(Message) {UUtuPluginLog::AddToLog(Message, EUtuLog::Error);}
#define UTU_LOG_CLEAR() {UUtuPluginLog::ClearLog();}
#define UTU_LOG_EMPTY_LINE() {UUtuPluginLog::AddToLog("", EUtuLog::Log);}
#define UTU_LOG_SEPARATOR_LINE() {UTU_LOG_EMPTY_LINE();UUtuPluginLog::AddToLog("---------------------------------------------------------", EUtuLog::Log);UTU_LOG_EMPTY_LINE();}
#define UTU_LOG_SEMI_SEPARATOR_LINE() {UUtuPluginLog::AddToLog("----------------------------", EUtuLog::Log);}

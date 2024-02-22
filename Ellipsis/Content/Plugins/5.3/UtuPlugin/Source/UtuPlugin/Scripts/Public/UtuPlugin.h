// Copyright Alex Quevillon. All Rights Reserved.

#pragma once
#include "UtuPlugin/Scripts/Public/UtuPluginJson.h"
#include "UtuPlugin/Scripts/Public/UtuPluginAssets.h"

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "Editor/UnrealEd/Public/TickableEditorObject.h"
#include "UtuPlugin.generated.h"

struct FUtuPluginImportSettings;

USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuPluginCurrentImport {
	GENERATED_USTRUCT_BODY()
public:
	void Import(FUtuPluginJson Json, TArray<EUtuAssetType> AssetTypes, bool executeFullImportOnSameFrame, bool SaveAllAfterProcess, bool DeleteInvalidAssets);
	void BeginImport(FUtuPluginJson Json, TArray<EUtuAssetType> AssetTypes, bool SaveAllAfterProcess, bool DeleteInvalidAssets);
	bool ContinueImport(bool executeFullImportOnSameFrame);
	void CompleteImport();
	FString AssetTypeToString(EUtuAssetType AssetType);

public:
	// Global
	FUtuPluginJson json;
	FString timestamp = "";
	// Delayed Specific
	bool bSaveAllAfterProcess = false;
	bool bDeleteInvalidAssets = false;
	TArray<EUtuAssetType> assetTypesToProcess;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		FUtuPluginAssetTypeProcessor currentAssetTypeProcessor;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		bool bIsValid = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		int countAssetTypesToProcess = 1;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		int amountAssetTypesToProcess = 0;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		float percentAssetTypesToProcess = 0.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		FString namEUtuAssetTypesToProcess = "";
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		FUtuPluginImportSettings ImportSettings;
};

class FTick : public FTickableEditorObject {
protected:
	/** FTickableEditorObject interface */
	virtual void Tick(float DeltaTime);
	virtual ETickableTickType GetTickableTickType() const { return ETickableTickType::Always; }
	virtual TStatId GetStatId() const override;
};


UCLASS()
class UTUPLUGIN_API UUtuPlugin : public UBlueprintFunctionLibrary {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static void Import(FUtuPluginJson Json, TArray<EUtuAssetType> AssetTypes, bool executeFullImportOnSameFrame, bool SaveAllAfterProcess, bool DeleteInvalidAssets);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static void CancelImport();
	
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static FUtuPluginCurrentImport GetCurrentImportState();

	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static void SetImportSettings(FUtuPluginImportSettings ImportSettings);

	UFUNCTION(BlueprintPure, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static FString GetUtuPluginVersion();

	UFUNCTION(BlueprintPure, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static FString GetUtuPluginInfo();

	static FUtuPluginCurrentImport ContinueCurrentImport();
private:
	static FUtuPluginCurrentImport currentImportJob;
	static FUtuPluginImportSettings currentImportSettings;
};

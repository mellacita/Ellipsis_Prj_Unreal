// Copyright Alex Quevillon. All Rights Reserved.

#pragma once
#include "UtuPlugin/Scripts/Public/UtuPluginJson.h"
#include "CoreMinimal.h"
#include "Factories/FbxMeshImportData.h"
#include "Engine/Texture.h"
#include "UtuPluginAssets.generated.h"

class FAssetToolsModule;
class UAssetImportTask;
class USceneComponent;
class UBlueprint;
class USCS_Node;
class UPackage;
class ACineCamera;
class UCineCameraComponent;
class UMaterialExpressionTextureSampleParameter2D;
class UMaterialExpressionScalarParameter;
class UMaterialExpressionVectorParameter;
class UMaterial;
class UTexture;
class UMaterialExpressionMultiply;
class UMaterialExpressionPanner;
class UMaterialExpressionTextureCoordinate;
class UMaterialExpressionComponentMask;

USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuPluginImportSettings {
	GENERATED_USTRUCT_BODY()
public:
	// Static Mesh
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		bool bImportSeparated = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		bool bAutoGenerateCollision = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		bool bImportLods = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		bool bGenerateLightmapUVs = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		bool bRemoveDegenerates = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		TEnumAsByte<enum EFBXNormalImportMethod> NormalImportMethod;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		TEnumAsByte<enum EFBXNormalGenerationMethod::Type> NormalGenerationMethod;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		bool bComputeWeightedNormals = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		float MeshImportScaleMultiplierIfUseFileScale = 1.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		float MeshImportScaleMultiplierIfNotUseFileScale = 1.0f;
	// Skeletal Mesh
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		bool bUseT0AsRefPose = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		bool bPreserveSmoothingGroups = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		bool bImportMeshesInBoneHierarchy = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		bool bImportMorphTargets = true;
	// Textures
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		TEnumAsByte<enum TextureCompressionSettings> CompressionSettings;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		TEnumAsByte<enum TextureFilter> Filter;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		TEnumAsByte<enum TextureGroup> LODGroup;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		bool SRGB = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		int32 MaxTextureSize;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		TEnumAsByte<enum ETextureCompressionQuality> CompressionQuality;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		TEnumAsByte<enum TextureMipGenSettings> MipGenSettings;
	// Lights
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		float SkyLightIntensity = 1.5f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		float LightIntensityMultiplier = 10.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		float LightRangeMultiplier = 100.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		float LightFalloffExponent = 2.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		float LightSpotInnerConeAngle = 0.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		float LightSpotAngleMultiplier = 0.5f;
	// Materials
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		bool bCreateMaterialInstances = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		float TexturesPannerTime = 5.45f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		float MetallicIntensityMultiplier = 1.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		float SpecularIntensityMultiplier = 1.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		float EmissiveIntensityMultiplier = 4.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		float NormalIntensityMultiplier = 1.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		float OcclusionIntensityMultiplier = 1.0f;
};

USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuPluginAssetTypeProcessor {
	GENERATED_USTRUCT_BODY()
public:
	void Import(FUtuPluginJson Json, EUtuAssetType AssetType, bool executeFullImportOnSameFrame, bool DeleteInvalidAssets);
	void BeginImport(FUtuPluginJson Json, EUtuAssetType AssetType, bool DeleteInvalidAssets);
	bool ContinueImport();
	void CompleteImport();

public:
	int GetAssetsNum();
private:
	TArray<FString> FormatRelativeFilenameForUnreal(FString InRelativeFilename, bool bInIsMaterial = false); //[0] = path, [1] = name, [2] = relative filename
	void ProcessScene(FUtuPluginScene InUtuScene);
	void ProcessMesh(FUtuPluginMesh InUtuMesh);
	void ProcessMaterial(FUtuPluginMaterial InUtuMaterial);
	class UMaterial* GetOrCreateParentMaterial(FUtuPluginMaterial InUtuMaterial);
	void ProcessTexture(FUtuPluginTexture InUtuTexture);
	void ProcessPrefabFirstPass(FUtuPluginPrefabFirstPass InUtuPrefabFirstPass);
	void ProcessPrefabSecondPass(FUtuPluginPrefabSecondPass InUtuPrefabSecondPass);
	UAssetImportTask* BuildTask(FString InSource, TArray<FString> InAssetNames, UObject* InOptions);

	TArray<FString> StartProcessAsset(FUtuPluginAsset InUtuAsset, bool bInIsMaterial = false);
	bool DeleteInvalidAssetIfNeeded(TArray<FString> InAssetNames, UClass* InClass);

	UPackage* CreateAssetPackage(FString InRelativeFilename, bool bLoadPackage);
	void LogAssetCreateOrNot(UObject* InAsset);
	void LogAssetImportOrReimport(UObject* InAsset);
	void LogAssetImportedOrFailed(UObject* InAsset, TArray<FString> InAssetNames, FString InSourceFileFullname, FString InAssetType, TArray<FString> InPotentialCauses);

	AActor* WorldAddRootActorForSubActorsIfNeeded(UWorld* InAsset, FUtuPluginActor InUtuActor);
	AActor* WorldSpawnStaticMeshActor(UWorld* InAsset, FUtuPluginActor InUtuActor);
	AActor* WorldSpawnSkeletalMeshActor(UWorld* InAsset, FUtuPluginActor InUtuActor);
	AActor* WorldSpawnBlueprintActor(UWorld* InAsset, FUtuPluginActor InUtuActor);
	AActor* WorldSpawnSkyLightActor(UWorld* InAsset);
	AActor* WorldSpawnPointLightActor(UWorld* InAsset, FUtuPluginActor InUtuActor);
	AActor* WorldSpawnDirectionalLightActor(UWorld* InAsset, FUtuPluginActor InUtuActor);
	AActor* WorldSpawnSpotLightActor(UWorld* InAsset, FUtuPluginActor InUtuActor);
	AActor* WorldSpawnCameraActor(UWorld* InAsset, FUtuPluginActor InUtuActor);

	void BpAddRootComponent(UBlueprint* InAsset, bool bStatic);
	bool BpAddRootComponentForSubComponentsIfNeeded(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode);
	void BpAddEmptyComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated);
	void BpAddStaticMeshComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated);
	void BpAddSkeletalMeshComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated);
	void BpAddPointLightComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated);
	void BpAddDirectionalLightComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated);
	void BpAddSpotLightComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated);
	void BpAddCameraComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated);
	void BpAddChildActorComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated);

	FString BpMakeUniqueName(FString InDesiredName, TArray<FString>& InOutUsedNames);
	
	FLinearColor HexToColor(FString InHex);
	UTexture2D* GetTextureFromUnityRelativeFilename(FString InUnityRelativeFilename);
	UMaterialExpressionTextureSampleParameter2D* GetOrCreateTextureParameter(UMaterial* InMaterial, UTexture* InTexture, FName InParamName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial);
	UMaterialExpressionScalarParameter* GetOrCreateScalarParameter(UMaterial* InMaterial, float InValue, FName InParamName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial);
	UMaterialExpressionVectorParameter* GetOrCreateVectorParameter(UMaterial* InMaterial, FLinearColor InColor, FName InParamName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial);
	UMaterialExpressionComponentMask* GetOrCreateMaskExpression(UMaterial* InMaterial, FString InExpressionName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial);
	UMaterialExpressionMultiply* GetOrCreateMultiplyExpression(UMaterial* InMaterial, FString InExpressionName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial);
	UMaterialExpressionPanner* GetOrCreatePannerExpression(UMaterial* InMaterial, FVector2D InValue, FString InExpressionName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial);
	UMaterialExpressionTextureCoordinate* GetOrCreateTexCoordExpression(UMaterial* InMaterial, FVector2D InValue, FString InExpressionName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial);
	
public:
	FAssetToolsModule* AssetTools;
	// Global
	FUtuPluginJson json;
	EUtuAssetType assetType;
	bool bDeleteInvalidAssets = false;
	// Delayed Specific
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		bool bIsValid = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		int countItemsToProcess = 1;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		int amountItemsToProcess = 0;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		float percentItemsToProcess = 0.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		FString nameItemToProcess = "";

	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		FUtuPluginImportSettings ImportSettings;
};
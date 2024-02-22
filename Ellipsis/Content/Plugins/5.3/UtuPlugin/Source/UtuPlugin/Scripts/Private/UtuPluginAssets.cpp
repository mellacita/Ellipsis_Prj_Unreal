// Copyright Alex Quevillon. All Rights Reserved.

#include "UtuPlugin/Scripts/Public/UtuPluginAssets.h"
#include "UtuPlugin/Scripts/Public/UtuPluginPaths.h"
#include "UtuPlugin/Scripts/Public/UtuPluginConstants.h"
#include "UtuPlugin/Scripts/Public/UtuPluginLog.h"
#include "UtuPlugin/Scripts/Public/UtuPluginLibrary.h"

#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"
#include "Editor/UnrealEd/Public/AssetImportTask.h"
#include "Editor/UnrealEd/Public/FileHelpers.h"
#include "Editor/UnrealEd/Classes/Factories/MaterialFactoryNew.h"
#include "Runtime/Engine/Classes/Materials/Material.h"
#include "ObjectTools.h"
#include "EditorUtilityLibrary.h"

#include "EditorAssetLibrary.h"

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/AssetRegistryInterface.h"
#else
#include "Runtime/AssetRegistry/Public/AssetRegistryModule.h"
#include "Runtime/AssetRegistry/Public/IAssetRegistry.h"
#endif

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 2
#include "Engine/SkinnedAssetCommon.h"
#endif

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "Runtime/Engine/Public/ComponentReregisterContext.h"
#include "Editor/UnrealEd/Classes/Factories/FbxImportUI.h"
#include "Editor/UnrealEd/Classes/Factories/FbxStaticMeshImportData.h"
#include "Editor/UnrealEd/Classes/Factories/FbxSkeletalMeshImportData.h"
//#include "Edutor/UnrealEd/Classes/Factories/FbxAnimSequenceImportData.h" /// TODO : Support animations
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "Runtime/Engine/Classes/Engine/SkeletalMesh.h"
#include "Editor/UnrealEd/Public/Kismet2/KismetEditorUtilities.h"
#include "Editor/KismetCompiler/Public/KismetCompilerModule.h"
#include "Runtime/Engine/Classes/Engine/SimpleConstructionScript.h"
#include "Runtime/Engine/Classes/Engine/SCS_Node.h"
#include "Runtime/Engine/Classes/Animation/AnimSequence.h"
//#include "Editor/UnrealEd/Public/Toolkits/AssetEditorManager.h"
//#include "Editor/Kismet/Public/BlueprintEditor.h"
#include "Editor/UnrealEd/Classes/Editor/EditorEngine.h"
#include "Editor/UnrealEd/Classes/Factories/WorldFactory.h"
#include "Editor/UnrealEd/Classes/ActorFactories/ActorFactoryEmptyActor.h"
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"
#include "Runtime/Engine/Classes/Animation/SkeletalMeshActor.h"
#include "BlueprintEditorSettings.h"
//#include "Editor/BlueprintGraph/Public/BlueprintEditorImportSettings.h"
#include "Editor/UnrealEd/Public/Kismet2/BlueprintEditorUtils.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Materials/MaterialExpressionComponentMask.h"
#include "Runtime/Engine/Classes/Engine/DirectionalLight.h"
#include "Runtime/Engine/Classes/Engine/PointLight.h"
#include "Runtime/Engine/Classes/Engine/SpotLight.h"
#include "Runtime/Engine/Classes/Engine/SkyLight.h"
#include "Runtime/Engine/Classes/Components/LightComponent.h"
#include "Runtime/Engine/Classes/Components/PointLightComponent.h"
#include "Runtime/Engine/Classes/Components/SpotLightComponent.h"
#include "Runtime/Engine/Classes/Components/DirectionalLightComponent.h"
#include "Runtime/Engine/Classes/Components/SkyLightComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Runtime/CinematicCamera/Public/CineCameraActor.h"
#include "Runtime/CinematicCamera/Public/CineCameraComponent.h"
#include "Runtime/Engine/Classes/Camera/CameraActor.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionPanner.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Engine/TextureCube.h"
#include "MaterialEditingLibrary.h"

#include "Runtime/Launch/Resources/Version.h" 

void FUtuPluginAssetTypeProcessor::Import(FUtuPluginJson Json, EUtuAssetType AssetType, bool executeFullImportOnSameFrame, bool DeleteInvalidAssets) {
	BeginImport(Json, AssetType, DeleteInvalidAssets);
	if (executeFullImportOnSameFrame) {
		while (ContinueImport() != true) {
			// ContinueImport
		}
	}
}

void FUtuPluginAssetTypeProcessor::BeginImport(FUtuPluginJson Json, EUtuAssetType AssetType, bool DeleteInvalidAssets) {
	AssetTools = FModuleManager::LoadModulePtr<FAssetToolsModule>("AssetTools");
	json = Json;
	assetType = AssetType;
	bDeleteInvalidAssets = DeleteInvalidAssets;
	amountItemsToProcess = GetAssetsNum();
	countItemsToProcess = 1;
	percentItemsToProcess = (float)countItemsToProcess / (float)amountItemsToProcess;
	if (amountItemsToProcess == 0) {
		bIsValid = false;
	}
}

bool FUtuPluginAssetTypeProcessor::ContinueImport() {
	if (GetAssetsNum() == 0) {
		//UTU_LOG_E("FUtuPluginAssetTypeProcessor::ContinueImport() Was called even though the list is already empty. This should never happen!");
		CompleteImport();
		return true;
	}
	switch (assetType) {
	case EUtuAssetType::Scene:
		nameItemToProcess = json.scenes[0].asset_name;
		ProcessScene(json.scenes[0]);
		json.scenes.RemoveAt(0);
		break;
	case EUtuAssetType::Mesh:
		nameItemToProcess = json.meshes[0].asset_name;
		ProcessMesh(json.meshes[0]);
		json.meshes.RemoveAt(0);
		break;
	case EUtuAssetType::Material:
		nameItemToProcess = json.materials[0].asset_name;
		ProcessMaterial(json.materials[0]);
		json.materials.RemoveAt(0);
		break;
	case EUtuAssetType::Texture:
		nameItemToProcess = json.textures[0].asset_name;
		ProcessTexture(json.textures[0]);
		json.textures.RemoveAt(0);
		break;
	case EUtuAssetType::PrefabFirstPass:
		nameItemToProcess = json.prefabs_first_pass[0].asset_name;
		ProcessPrefabFirstPass(json.prefabs_first_pass[0]);
		json.prefabs_first_pass.RemoveAt(0);
		break;
	case EUtuAssetType::PrefabSecondPass:
		nameItemToProcess = json.prefabs_second_pass[0].asset_name;
		ProcessPrefabSecondPass(json.prefabs_second_pass[0]);
		json.prefabs_second_pass.RemoveAt(0);
		break;
	default:
		break;
	}
	countItemsToProcess++;
	percentItemsToProcess = (float)countItemsToProcess / (float)FMath::Max(amountItemsToProcess, 1);
	if (GetAssetsNum() == 0) {
		CompleteImport();
		return true;
	}
	return false;
}

void FUtuPluginAssetTypeProcessor::CompleteImport() {

}

TArray<FString> FUtuPluginAssetTypeProcessor::FormatRelativeFilenameForUnreal(FString InRelativeFilename, bool bInIsMaterial) {
	if (InRelativeFilename != "") {
		FString Relative = InRelativeFilename;
		if (Relative.RemoveFromStart("Assets")) {
			Relative = "/Game" + Relative;
		}
		if (bInIsMaterial) {
			// Fix the fact that materials doesn't really exists in Unity if they are from the .fbx file -.-
			Relative = Relative.Replace(TEXT(".fbx"), TEXT("_Mat"));
			// Same when it come from a .mat file --.--
			Relative = Relative.Replace(TEXT(".mat"), TEXT("_Mat"));
		}
		Relative = Relative.Replace(*UtuPluginPaths::backslash, *UtuPluginPaths::slash);
		Relative = Relative.Replace(TEXT(" "), TEXT("_"));
		if (Relative.Contains(".")) {
			Relative.Split(".", &Relative, nullptr, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		}
		if (!Relative.StartsWith("/Game")) {
			Relative = "/UtuPlugin/Default/" + Relative.Replace(TEXT("Resources"), TEXT(""));
			//Relative = "/Game/INVALID_FILENAME/" + Relative;
		}
		Relative = Relative.Replace(TEXT("."), TEXT("_")); // Dots in asset path? Really!? -.-
		Relative = Relative.Replace(TEXT("\""), TEXT("_"));
		Relative = Relative.Replace(TEXT("'"), TEXT("_"));
		Relative = Relative.Replace(TEXT(" "), TEXT("_"));
		Relative = Relative.Replace(TEXT(","), TEXT("_"));
		Relative = Relative.Replace(TEXT("."), TEXT("_"));
		Relative = Relative.Replace(TEXT(":"), TEXT("_"));
		Relative = Relative.Replace(TEXT("|"), TEXT("_"));
		Relative = Relative.Replace(TEXT("&"), TEXT("_"));
		Relative = Relative.Replace(TEXT("!"), TEXT("_"));
		Relative = Relative.Replace(TEXT("~"), TEXT("_"));
		Relative = Relative.Replace(TEXT("@"), TEXT("_"));
		Relative = Relative.Replace(TEXT("#"), TEXT("_"));
		Relative = Relative.Replace(TEXT("("), TEXT("_"));
		Relative = Relative.Replace(TEXT(")"), TEXT("_"));
		Relative = Relative.Replace(TEXT("{"), TEXT("_"));
		Relative = Relative.Replace(TEXT("}"), TEXT("_"));
		Relative = Relative.Replace(TEXT("["), TEXT("_"));
		Relative = Relative.Replace(TEXT("]"), TEXT("_"));
		Relative = Relative.Replace(TEXT("="), TEXT("_"));
		Relative = Relative.Replace(TEXT(";"), TEXT("_"));
		Relative = Relative.Replace(TEXT("^"), TEXT("_"));
		Relative = Relative.Replace(TEXT("%"), TEXT("_"));
		Relative = Relative.Replace(TEXT("$"), TEXT("_"));
		Relative = Relative.Replace(TEXT("`"), TEXT("_"));
		Relative = Relative.Replace(TEXT("*"), TEXT("_"));
		Relative = Relative.Replace(TEXT("?"), TEXT("_"));
		Relative = Relative.Replace(TEXT("+"), TEXT("_"));
		FString Path;
		FString Filename;
		Relative.Split("/", &Path, &Filename, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		return { Path, Filename, Path / Filename };
	}
	return {"", "", ""};
}

int FUtuPluginAssetTypeProcessor::GetAssetsNum() {
	switch (assetType) {
	case EUtuAssetType::Scene:
		return json.scenes.Num();
		break;
	case EUtuAssetType::Mesh:
		return json.meshes.Num();
		break;
	case EUtuAssetType::Material:
		return json.materials.Num();
		break;
	case EUtuAssetType::Texture:
		return json.textures.Num();
		break;
	case EUtuAssetType::PrefabFirstPass:
		return json.prefabs_first_pass.Num();
		break;
	case EUtuAssetType::PrefabSecondPass:
		return json.prefabs_second_pass.Num();
		break;
	default:
		break;
	}
	return 0;
}

void FUtuPluginAssetTypeProcessor::ProcessScene(FUtuPluginScene InUtuScene) {
	// Format Paths
	TArray<FString> AssetNames = StartProcessAsset(InUtuScene);
	// Invalid Asset
	if (DeleteInvalidAssetIfNeeded(AssetNames, UWorld::StaticClass())) {
		// Existing Asset
		UWorld* Asset = Cast<UWorld>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
		LogAssetCreateOrNot(Asset);
		// Create Asset
		if (Asset == nullptr) {
			UPackage* Package = CreateAssetPackage(AssetNames[2], true);
			UWorldFactory* Factory = NewObject<UWorldFactory>();
			Asset = Cast<UWorld>(Factory->FactoryCreateNew(UWorld::StaticClass(), Package, FName(*AssetNames[1]), RF_Public | RF_Standalone, NULL, GWarn));
			LogAssetImportedOrFailed(Asset, AssetNames, "", "World", { });
		}
		// Process Asset
		if (Asset != nullptr) {
			Asset->PreEditChange(NULL);
			// Delete Old Actors -- TODO : Analyse them and keep the good ones
			TArray<AActor*> OldActors;
			UGameplayStatics::GetAllActorsWithTag(Asset, "UtuActor", OldActors);
			if (OldActors.Num() > 0) {
				UTU_LOG_L("        " + FString::FromInt(OldActors.Num()) + " old actors from previous import detected. Deleting them...");
				//UTU_LOG_L("            Plan for a future release: Analyse existing Actors and keep the good ones.");
				for (AActor* OldActor : OldActors) {
					Asset->DestroyActor(OldActor);
				}
			}
			// Start by creating a simple SkyLight
			WorldSpawnSkyLightActor(Asset);
			// Map
			TMap<int, AActor*> IdToActor;
			TMap<AActor*, int> ActorToParentId;
			// Spawn Actors
			for (FUtuPluginActor UtuActor : InUtuScene.scene_actors) {
				AActor* RootActor = WorldAddRootActorForSubActorsIfNeeded(Asset, UtuActor);
				if (RootActor != nullptr) {
					RootActor->Tags.Add("UtuActor");
					IdToActor.Add(UtuActor.actor_id, RootActor);
					ActorToParentId.Add(RootActor, UtuActor.actor_parent_id);
				}
				// Spawn Real Actors
				for (EUtuActorType ActorType : UtuActor.actor_types) {
					AActor* Actor = nullptr;
					if (ActorType == EUtuActorType::Empty) {
						// Don't care about an empty actor 'cause it's already spawned as a RootActor
					}
					else if (ActorType == EUtuActorType::StaticMesh) {
						Actor = WorldSpawnStaticMeshActor(Asset, UtuActor);
					}
					else if (ActorType == EUtuActorType::SkeletalMesh) {
						Actor = WorldSpawnSkeletalMeshActor(Asset, UtuActor); // TODO : Support Skeletal Mesh
						//Actor = WorldSpawnStaticMeshActor(Asset, UtuActor);
					}
					else if (ActorType == EUtuActorType::PointLight) {
						Actor = WorldSpawnPointLightActor(Asset, UtuActor);
					}
					else if (ActorType == EUtuActorType::DirectionalLight) {
						Actor = WorldSpawnDirectionalLightActor(Asset, UtuActor);
					}
					else if (ActorType == EUtuActorType::SpotLight) {
						Actor = WorldSpawnSpotLightActor(Asset, UtuActor);
					}
					else if (ActorType == EUtuActorType::Camera) {
						Actor = WorldSpawnCameraActor(Asset, UtuActor);
					}
					else if (ActorType == EUtuActorType::Prefab) {
						Actor = WorldSpawnBlueprintActor(Asset, UtuActor);
					}
					// Attachment
					if (Actor != nullptr) {
						Actor->GetRootComponent()->SetMobility(UtuActor.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
						if (RootActor == nullptr) {
							Actor->Tags.Add("UtuActor");
							Actor->SetActorLabel(UtuActor.actor_display_name);
							Actor->Tags.Add(*FString::FromInt(UtuActor.actor_id));
							if (UtuActor.actor_tag != "Untagged") {
								Actor->Tags.Add(*UtuActor.actor_tag);
							}
							Actor->SetActorHiddenInGame(!UtuActor.actor_is_visible);
							if (ActorType == EUtuActorType::StaticMesh && ImportSettings.bImportSeparated)
							{
								Actor->SetActorLocation(UtuConst::ConvertLocation(UtuActor.actor_world_location_if_separated));
							}
							else
							{
								Actor->SetActorLocation(UtuConst::ConvertLocation(UtuActor.actor_world_location));
							}
							Actor->SetActorRotation(UtuConst::ConvertRotation(UtuActor.actor_world_rotation));
							Actor->SetActorScale3D(UtuConst::ConvertScale(UtuActor.actor_world_scale));
							IdToActor.Add(UtuActor.actor_id, Actor);
							ActorToParentId.Add(Actor, UtuActor.actor_parent_id);
						}
						else {
							Actor->AttachToActor(RootActor, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
						}
					}
				}
			}
			// Parent Actors
			UTU_LOG_L("        Parenting actors...");
			TArray<AActor*> Keys;
			ActorToParentId.GetKeys(Keys);
			for (AActor* Actor : Keys) {
				int Id = ActorToParentId[Actor];
				if (Id != UtuConst::INVALID_INT) {
					if (IdToActor.Contains(Id)) {
						AActor* ParentActor = IdToActor[Id];
						Actor->AttachToActor(ParentActor, FAttachmentTransformRules(EAttachmentRule::KeepWorld, false));
						UTU_LOG_L("            " + Actor->GetActorLabel() + " -> " + ParentActor->GetActorLabel());
					}
					else {
						UTU_LOG_W("            Failed to find parent for: " + Actor->GetActorLabel());
						UTU_LOG_W("                Potential Causes:");
						UTU_LOG_W("                    - The desired parent of this actor is a component of a prefab.");
						UTU_LOG_W("                      (Scene edits of prefabs' components aren't supported yet.)");
						UTU_LOG_W("                    - The desired parent failed to spawn for some reason. (Missing Bp asset maybe?)");
					}
				}
			}
			Asset->MarkPackageDirty();
			Asset->PostEditChange();
		}
	}
}

void FUtuPluginAssetTypeProcessor::ProcessMesh(FUtuPluginMesh InUtuMesh) {
	if (InUtuMesh.asset_relative_filename.StartsWith("Assets")) { // Default Unity Mesh
		// Format Paths
		TArray<FString> AssetNames = StartProcessAsset(InUtuMesh);
		if (InUtuMesh.is_skeletal_mesh) {
			// Invalid Asset
			if (DeleteInvalidAssetIfNeeded(AssetNames, USkeletalMesh::StaticClass())) {
				// Existing Asset
				USkeletalMesh* Asset = Cast<USkeletalMesh>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
				LogAssetImportOrReimport(Asset);
				// Create Asset
				UTU_LOG_L("        Fbx File Fullname: " + InUtuMesh.mesh_file_absolute_filename);
				UTU_LOG_L("        Import Scale Factor: " + FString::SanitizeFloat(InUtuMesh.mesh_import_scale_factor));
				UFbxImportUI* Options = NewObject<UFbxImportUI>();
				Options->bIsObjImport = InUtuMesh.mesh_file_absolute_filename.EndsWith(".obj", ESearchCase::IgnoreCase);
				Options->bAutomatedImportShouldDetectType = false;
				Options->bImportAnimations = false;
				Options->MeshTypeToImport = EFBXImportType::FBXIT_SkeletalMesh;
				Options->bImportMesh = true;
				Options->bImportTextures = false;
				Options->bImportMaterials = false;
				Options->bImportAsSkeletal = true;
				Options->SkeletalMeshImportData->NormalImportMethod = ImportSettings.NormalImportMethod;
				Options->SkeletalMeshImportData->NormalGenerationMethod = ImportSettings.NormalGenerationMethod;
				Options->SkeletalMeshImportData->bComputeWeightedNormals = ImportSettings.bComputeWeightedNormals;
				Options->SkeletalMeshImportData->bUseT0AsRefPose = ImportSettings.bUseT0AsRefPose;
				Options->SkeletalMeshImportData->bPreserveSmoothingGroups = ImportSettings.bPreserveSmoothingGroups;
				Options->SkeletalMeshImportData->bImportMeshesInBoneHierarchy = ImportSettings.bImportMeshesInBoneHierarchy;
				Options->SkeletalMeshImportData->bImportMorphTargets = ImportSettings.bImportMorphTargets;
				Options->SkeletalMeshImportData->bComputeWeightedNormals = ImportSettings.bComputeWeightedNormals;
				Options->SkeletalMeshImportData->bImportMeshLODs = ImportSettings.bImportLods;
				Options->SkeletalMeshImportData->bTransformVertexToAbsolute = true;
				Options->SkeletalMeshImportData->bConvertScene = true;
				Options->SkeletalMeshImportData->bForceFrontXAxis = true;
				Options->SkeletalMeshImportData->bConvertSceneUnit = true;
				Options->SkeletalMeshImportData->ImportTranslation = UtuConst::ConvertLocation(InUtuMesh.mesh_import_position_offset, true, InUtuMesh.mesh_import_rotation_offset) / FMath::Max(0.0001f, InUtuMesh.mesh_import_scale_offset.X);
				Options->SkeletalMeshImportData->ImportRotation = FRotator(UtuConst::ConvertRotation(InUtuMesh.mesh_import_rotation_offset, true));
				Options->SkeletalMeshImportData->ImportUniformScale = InUtuMesh.mesh_import_scale_factor / FMath::Max(0.0001f, InUtuMesh.mesh_import_scale_offset.X);
				AssetTools->Get().ImportAssetTasks({ BuildTask(InUtuMesh.mesh_file_absolute_filename, AssetNames, Options) });
				Asset = Cast<USkeletalMesh>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
				LogAssetImportedOrFailed(Asset, AssetNames, InUtuMesh.mesh_file_absolute_filename, "SkeletalMesh", { "Invalid FBX : Make sure that the Fbx file is valid by trying to import it manually in Unreal." });
				// Process Asset
				if (Asset != nullptr) {
					Asset->PreEditChange(NULL);
					UTU_LOG_L("                Associating Materials to Skeletal Mesh...");
					Asset->Materials.Empty();
					for (int x = 0; x < InUtuMesh.mesh_materials_relative_filenames.Num(); x++) {
						TArray<FString> MatNames = FormatRelativeFilenameForUnreal(InUtuMesh.mesh_materials_relative_filenames[x], true);
						UTU_LOG_L("                    MaterialId[" + FString::FromInt(x) + "] : " + MatNames[2]);
						UMaterialInterface* MaterialAsset = Cast<UMaterialInterface>(UUtuPluginLibrary::TryGetAsset(MatNames[2]));
						Asset->Materials.Add(MaterialAsset);
						if (MaterialAsset == nullptr) {
							UTU_LOG_W("                        Failed to assign material because it doesn't exists: '" + MatNames[2] + "'");
						}
					}
					Asset->PostEditChange();
					// TODO : Support Animations
					//if (InUtuMesh.skeletal_mesh_animations_relative_filenames.Num() > 0) {
					//	UTU_LOG_L("                Importing Animations Associated with Skeletal Mesh...");
					//	for (FString x : InUtuMesh.skeletal_mesh_animations_relative_filenames) {
					//		TArray<FString> AnimNames = FormatRelativeFilenameForUnreal(x, false);
					//		// Invalid Asset
					//		if (DeleteInvalidAssetIfNeeded(AssetNames, UAnimSequence::StaticClass())) {
					//			// Existing Asset
					//			UAnimSequence* AnimAsset = Cast<UAnimSequence>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
					//			// Create Asset
					//			UTU_LOG_L("                    Fbx File Fullname: " + AnimNames[2]);
					//			UFbxImportUI* Options = NewObject<UFbxImportUI>();
					//			Options->bAutomatedImportShouldDetectType = false;
					//			Options->bImportAnimations = true;
					//			Options->MeshTypeToImport = EFBXImportType::FBXIT_Animation;
					//			Options->bImportMesh = false;
					//			Options->bImportTextures = false;
					//			Options->bImportMaterials = false;
					//			Options->bImportAsSkeletal = false;
					//			Options->AnimSequenceImportData->
					//			AssetTools->Get().ImportAssetTasks({ BuildTask(InUtuMesh.mesh_file_absolute_filename, AssetNames, Options) });
					//			AnimAsset = Cast<USkeletalMesh>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
					//			LogAssetImportedOrFailed(AnimAsset, AssetNames, InUtuMesh.mesh_file_absolute_filename, "SkeletalMesh", { "Invalid FBX : Make sure that the Fbx file is valid by trying to import it manually in Unreal." });
					//		}
					//	}
					//}
				}
			}
		}
		else {
			// Invalid Asset
			if (DeleteInvalidAssetIfNeeded(AssetNames, UStaticMesh::StaticClass())) {
				// Existing Asset
				UStaticMesh* Asset = Cast<UStaticMesh>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
				if (Asset == nullptr && ImportSettings.bImportSeparated) {
					// Try separated way
					Asset = Cast<UStaticMesh>(UUtuPluginLibrary::TryGetAsset(AssetNames[2] + "_" + InUtuMesh.asset_name.Replace(TEXT(" "), TEXT("_")).Replace(TEXT(".fbx"), TEXT(""))));
				}
				LogAssetImportOrReimport(Asset);
				// Create Asset
				UTU_LOG_L("        Fbx File Fullname: " + InUtuMesh.mesh_file_absolute_filename);
				UTU_LOG_L("        Import Scale Factor: " + FString::SanitizeFloat(InUtuMesh.mesh_import_scale_factor));
				UFbxImportUI* Options = NewObject<UFbxImportUI>();
				Options->bIsObjImport = InUtuMesh.mesh_file_absolute_filename.EndsWith(".obj", ESearchCase::IgnoreCase);
				Options->bAutomatedImportShouldDetectType = false;
				Options->bImportAnimations = false;
				Options->MeshTypeToImport = EFBXImportType::FBXIT_StaticMesh;
				Options->bImportMesh = true;
				Options->bImportTextures = false;
				Options->bImportMaterials = false;
				Options->bImportAsSkeletal = false; 
				if (ImportSettings.bImportSeparated)
				{
					Options->StaticMeshImportData->bCombineMeshes = false;
					Options->StaticMeshImportData->bTransformVertexToAbsolute = true;
				}
				else
				{
					Options->StaticMeshImportData->bCombineMeshes = true;
					Options->StaticMeshImportData->bTransformVertexToAbsolute = false;
				}
				Options->StaticMeshImportData->bBakePivotInVertex = false;
				Options->StaticMeshImportData->bTransformVertexToAbsolute = true;
				Options->StaticMeshImportData->bConvertScene = true;
				Options->StaticMeshImportData->bForceFrontXAxis = true;
				Options->StaticMeshImportData->bConvertSceneUnit = true;
				Options->StaticMeshImportData->bAutoGenerateCollision = ImportSettings.bAutoGenerateCollision;
				Options->StaticMeshImportData->bGenerateLightmapUVs = ImportSettings.bGenerateLightmapUVs;
				Options->StaticMeshImportData->bRemoveDegenerates = ImportSettings.bRemoveDegenerates;
				Options->StaticMeshImportData->NormalImportMethod = ImportSettings.NormalImportMethod;
				Options->StaticMeshImportData->NormalGenerationMethod = ImportSettings.NormalGenerationMethod;
				Options->StaticMeshImportData->bComputeWeightedNormals = ImportSettings.bComputeWeightedNormals;
				Options->StaticMeshImportData->ImportTranslation = UtuConst::ConvertLocation(InUtuMesh.mesh_import_position_offset, true, InUtuMesh.mesh_import_rotation_offset) / FMath::Max(0.0001f, InUtuMesh.mesh_import_scale_offset.X);
				Options->StaticMeshImportData->ImportRotation = FRotator(UtuConst::ConvertRotation(InUtuMesh.mesh_import_rotation_offset, true));
				float Scale = InUtuMesh.mesh_import_scale_factor * (InUtuMesh.use_file_scale ? ImportSettings.MeshImportScaleMultiplierIfUseFileScale : ImportSettings.MeshImportScaleMultiplierIfNotUseFileScale);
				Options->StaticMeshImportData->ImportUniformScale = Scale / FMath::Max(0.0001f, InUtuMesh.mesh_import_scale_offset.X);
				//Options->StaticMeshImportData->ImportUniformScale = FMath::Max(0.0001f, InUtuMesh.mesh_import_scale_offset.X);
				//if (!InUtuMesh.mesh_import_convert_units)
				//{
				//	Options->StaticMeshImportData->ImportUniformScale = (InUtuMesh.mesh_import_scale_factor / Options->StaticMeshImportData->ImportUniformScale) * ImportSettings.ScaleMultiplierForNonConvertUnitMeshes;
				//}
				AssetTools->Get().ImportAssetTasks({ BuildTask(InUtuMesh.mesh_file_absolute_filename, AssetNames, Options) });
				Asset = Cast<UStaticMesh>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
				if (Asset == nullptr && ImportSettings.bImportSeparated) {
					// Try separated way
					Asset = Cast<UStaticMesh>(UUtuPluginLibrary::TryGetAsset(AssetNames[2] + "_" + InUtuMesh.asset_name.Replace(TEXT(" "), TEXT("_")).Replace(TEXT(".fbx"), TEXT(""))));
				}
				LogAssetImportedOrFailed(Asset, AssetNames, InUtuMesh.mesh_file_absolute_filename, "StaticMesh", { "Invalid FBX : Make sure that the Fbx file is valid by trying to import it manually in Unreal." });
				// Process Asset
				if (Asset != nullptr) {
					UTU_LOG_L("                Associating Materials to Static Mesh...");
					for (int x = 0; x < InUtuMesh.mesh_materials_relative_filenames.Num(); x++) {
						TArray<FString> MatNames = FormatRelativeFilenameForUnreal(InUtuMesh.mesh_materials_relative_filenames[x], true);
						UTU_LOG_L("                    MaterialId[" + FString::FromInt(x) + "] : " + MatNames[2]);
						UMaterialInterface* MaterialAsset = Cast<UMaterialInterface>(UUtuPluginLibrary::TryGetAsset(MatNames[2]));
						Asset->SetMaterial(x, MaterialAsset);
						if (MaterialAsset == nullptr) {
							UTU_LOG_W("                        Failed to assign material because it doesn't exists: '" + MatNames[2] + "'");
						}
					}
				}
			}
		}
	}
}

UPackage* FUtuPluginAssetTypeProcessor::CreateAssetPackage(FString InRelativeFilename, bool bLoadPackage) {
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
	UPackage* RetPackage = CreatePackage(*InRelativeFilename);
#else
	UPackage* RetPackage = CreatePackage(NULL, *InRelativeFilename);
#endif
	RetPackage->MarkPackageDirty();
	if (bLoadPackage) {
		RetPackage->FullyLoad();
	}
	return RetPackage;
}

void FUtuPluginAssetTypeProcessor::ProcessMaterial(FUtuPluginMaterial InUtuMaterial) {
	if (!InUtuMaterial.asset_relative_filename.StartsWith("Resources")) // Default Unity Material
	{ 
		// Format Paths
		TArray<FString> AssetNames = StartProcessAsset(InUtuMaterial, true);
		// Invalid Asset
		if (ImportSettings.bCreateMaterialInstances)
		{
			if (DeleteInvalidAssetIfNeeded(AssetNames, UMaterialInstanceConstant::StaticClass()))
			{
				// Existing Asset
				UMaterialInstanceConstant* Asset = Cast<UMaterialInstanceConstant>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
				LogAssetCreateOrNot(Asset);

				UMaterial* ParentMaterial = GetOrCreateParentMaterial(InUtuMaterial);

				if (Asset == nullptr)
				{
					UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
					Factory->InitialParent = ParentMaterial;
					AssetTools->Get().CreateAsset(AssetNames[1], AssetNames[0], UMaterialInstanceConstant::StaticClass(), Factory);
					Asset = Cast<UMaterialInstanceConstant>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
					LogAssetImportedOrFailed(Asset, AssetNames, "", "MaterialInstance", { });
				}


				if (Asset != nullptr)
				{
					// Parent
					Asset->Parent = ParentMaterial;

					// Two Sided
					Asset->BasePropertyOverrides.bOverride_TwoSided = true;
					Asset->BasePropertyOverrides.TwoSided = InUtuMaterial.two_sided;

					// Blend Mode
					Asset->BasePropertyOverrides.bOverride_BlendMode = true;
					switch (InUtuMaterial.shader_opacity)
					{
					default:
						break;
					case EUtuShaderOpacity::Opaque:
						Asset->BasePropertyOverrides.BlendMode = EBlendMode::BLEND_Opaque;
						break;
					case EUtuShaderOpacity::Masked:
						Asset->BasePropertyOverrides.BlendMode = EBlendMode::BLEND_Masked;
						break;
					case EUtuShaderOpacity::Translucent:
						Asset->BasePropertyOverrides.BlendMode = EBlendMode::BLEND_Translucent;
						break;
					}

					// Settings
					Asset->SetScalarParameterValueEditorOnly(FName("Utu_MetallicIntensityMultiplier"), ImportSettings.MetallicIntensityMultiplier);
					Asset->SetScalarParameterValueEditorOnly(FName("Utu_SpecularIntensityMultiplier"), ImportSettings.SpecularIntensityMultiplier);
					Asset->SetScalarParameterValueEditorOnly(FName("Utu_EmissiveIntensityMultiplier"), ImportSettings.EmissiveIntensityMultiplier);
					Asset->SetScalarParameterValueEditorOnly(FName("Utu_NormalIntensityMultiplier"), ImportSettings.NormalIntensityMultiplier);
					Asset->SetScalarParameterValueEditorOnly(FName("Utu_OcclusionIntensityMultiplier"), ImportSettings.OcclusionIntensityMultiplier);
					Asset->SetScalarParameterValueEditorOnly(FName("Utu_TexturesPannerTime"), ImportSettings.TexturesPannerTime);

					// Floats
					for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_floats_names.Num(), InUtuMaterial.material_floats.Num()); Idx++)
					{
						FString Name = InUtuMaterial.material_floats_names[Idx];
						float Value = InUtuMaterial.material_floats[Idx];
						Asset->SetScalarParameterValueEditorOnly(*Name, Value);
					}

					// Textures
					for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_textures_names.Num(), InUtuMaterial.material_textures.Num()); Idx++)
					{
						FString Name = InUtuMaterial.material_textures_names[Idx];
						UTexture2D* Value = GetTextureFromUnityRelativeFilename(InUtuMaterial.material_textures[Idx]);
						if (Value != nullptr)
						{
							Asset->SetTextureParameterValueEditorOnly(*Name, Value);
						}
						Asset->SetScalarParameterValueEditorOnly(FName("Utu_TexturesPannerTime_" + Name), ImportSettings.TexturesPannerTime);
					}

					// Colors
					for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_colors_names.Num(), InUtuMaterial.material_colors.Num()); Idx++)
					{
						FString Name = InUtuMaterial.material_colors_names[Idx];
						FLinearColor Value = HexToColor(InUtuMaterial.material_colors[Idx]);
						Asset->SetVectorParameterValueEditorOnly(*Name, Value);
					}

					// Vectors
					for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_vectors_names.Num(), InUtuMaterial.material_vectors.Num()); Idx++)
					{
						FString Name = InUtuMaterial.material_vectors_names[Idx];
						FQuat Value = InUtuMaterial.material_vectors[Idx];
						Asset->SetVectorParameterValueEditorOnly(*Name, FLinearColor(Value.X, Value.Y, Value.Z, Value.W));
					}

					// Vector2s
					for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_vector2s_names.Num(), InUtuMaterial.material_vector2s.Num()); Idx++)
					{
						FString Name = InUtuMaterial.material_vector2s_names[Idx];
						FVector2D Value = InUtuMaterial.material_vector2s[Idx];
						Asset->SetVectorParameterValueEditorOnly(*Name, FLinearColor(Value.X, Value.Y, 0.0f, 0.0f));
					}

					// Ints
					for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_ints_names.Num(), InUtuMaterial.material_ints.Num()); Idx++)
					{
						FString Name = InUtuMaterial.material_ints_names[Idx];
						int Value = InUtuMaterial.material_ints[Idx];
						Asset->SetScalarParameterValueEditorOnly(*Name, Value);
					}
				}
			}
		}
		else
		{
			if (DeleteInvalidAssetIfNeeded(AssetNames, UMaterial::StaticClass()))
			{
				// Existing Asset
				UMaterial* MatAsset = Cast<UMaterial>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
				LogAssetCreateOrNot(MatAsset);

				UMaterial* ParentMaterial = GetOrCreateParentMaterial(InUtuMaterial);

				if (MatAsset == nullptr)
				{
					UEditorAssetLibrary::DuplicateLoadedAsset(ParentMaterial, AssetNames[2]);
					MatAsset = Cast<UMaterial>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
					LogAssetImportedOrFailed(MatAsset, AssetNames, "", "Material", { });
				}

				if (MatAsset != nullptr)
				{
					// Two Sided
					MatAsset->TwoSided = InUtuMaterial.two_sided;

					// Blend Mode
					switch (InUtuMaterial.shader_opacity)
					{
					default:
						break;
					case EUtuShaderOpacity::Opaque:
						MatAsset->BlendMode = EBlendMode::BLEND_Opaque;
						break;
					case EUtuShaderOpacity::Masked:
						MatAsset->BlendMode = EBlendMode::BLEND_Masked;
						break;
					case EUtuShaderOpacity::Translucent:
						MatAsset->BlendMode = EBlendMode::BLEND_Translucent;
						break;
					}

					// Settings
					MatAsset->SetScalarParameterValueEditorOnly(FName("Utu_MetallicIntensityMultiplier"), ImportSettings.MetallicIntensityMultiplier);
					MatAsset->SetScalarParameterValueEditorOnly(FName("Utu_SpecularIntensityMultiplier"), ImportSettings.SpecularIntensityMultiplier);
					MatAsset->SetScalarParameterValueEditorOnly(FName("Utu_EmissiveIntensityMultiplier"), ImportSettings.EmissiveIntensityMultiplier);
					MatAsset->SetScalarParameterValueEditorOnly(FName("Utu_NormalIntensityMultiplier"), ImportSettings.NormalIntensityMultiplier);
					MatAsset->SetScalarParameterValueEditorOnly(FName("Utu_OcclusionIntensityMultiplier"), ImportSettings.OcclusionIntensityMultiplier);
					MatAsset->SetScalarParameterValueEditorOnly(FName("Utu_TexturesPannerTime"), ImportSettings.TexturesPannerTime);

					// Floats
					for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_floats_names.Num(), InUtuMaterial.material_floats.Num()); Idx++)
					{
						FString Name = InUtuMaterial.material_floats_names[Idx];
						float Value = InUtuMaterial.material_floats[Idx];
						MatAsset->SetScalarParameterValueEditorOnly(*Name, Value);
					}

					// Textures
					for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_textures_names.Num(), InUtuMaterial.material_textures.Num()); Idx++)
					{
						FString Name = InUtuMaterial.material_textures_names[Idx];
						UTexture2D* Value = GetTextureFromUnityRelativeFilename(InUtuMaterial.material_textures[Idx]);
						if (Value != nullptr)
						{
							MatAsset->SetTextureParameterValueEditorOnly(*Name, Value);
						}
						MatAsset->SetScalarParameterValueEditorOnly(FName("Utu_TexturesPannerTime_" + Name), ImportSettings.TexturesPannerTime);
					}

					// Colors
					for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_colors_names.Num(), InUtuMaterial.material_colors.Num()); Idx++)
					{
						FString Name = InUtuMaterial.material_colors_names[Idx];
						FLinearColor Value = HexToColor(InUtuMaterial.material_colors[Idx]);
						MatAsset->SetVectorParameterValueEditorOnly(*Name, Value);
					}

					// Vectors
					for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_vectors_names.Num(), InUtuMaterial.material_vectors.Num()); Idx++)
					{
						FString Name = InUtuMaterial.material_vectors_names[Idx];
						FQuat Value = InUtuMaterial.material_vectors[Idx];
						MatAsset->SetVectorParameterValueEditorOnly(*Name, FLinearColor(Value.X, Value.Y, Value.Z, Value.W));
					}

					// Vector2s
					for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_vector2s_names.Num(), InUtuMaterial.material_vector2s.Num()); Idx++)
					{
						FString Name = InUtuMaterial.material_vector2s_names[Idx];
						FVector2D Value = InUtuMaterial.material_vector2s[Idx];
						MatAsset->SetVectorParameterValueEditorOnly(*Name, FLinearColor(Value.X, Value.Y, 0.0f, 0.0f));
					}

					// Ints
					for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_ints_names.Num(), InUtuMaterial.material_ints.Num()); Idx++)
					{
						FString Name = InUtuMaterial.material_ints_names[Idx];
						int Value = InUtuMaterial.material_ints[Idx];
						MatAsset->SetScalarParameterValueEditorOnly(*Name, Value);
					}
				}
			}
		}
	}
}

UMaterial* FUtuPluginAssetTypeProcessor::GetOrCreateParentMaterial(FUtuPluginMaterial InUtuMaterial)
{
	FString MatName = InUtuMaterial.shader_name;
	MatName = MatName.Replace(TEXT(" "), TEXT(""));
	MatName = MatName.Replace(TEXT("."), TEXT("_"));
	MatName = MatName.Replace(TEXT("/"), TEXT("_"));
	MatName = MatName.Replace(TEXT("("), TEXT(""));
	MatName = MatName.Replace(TEXT(")"), TEXT(""));

	// Default material (Supported materials)
	FString MatDir = "/UtuPlugin/Shaders/";
	FString MatPath = MatDir + MatName;
	UMaterial* Material = Cast<UMaterial>(UUtuPluginLibrary::TryGetAsset(MatPath));
	if (Material != nullptr)
	{
		UTU_LOG_L("        Parent Material: \"" + MatPath + "\"");
		return Material;
	}

	// Custom material (Unsupported materials)
	MatDir = "/Game/UtuCustomShaders/";
	MatPath = MatDir + MatName;
	Material = Cast<UMaterial>(UUtuPluginLibrary::TryGetAsset(MatPath));
	if (Material != nullptr)
	{
		UTU_LOG_L("        Parent Material: \"" + MatPath + "\"");
		return Material;
	}


	// Create custom material
	UTU_LOG_W("        No Supported Material Found. Creating Custom Material: \"" + MatPath + "\" ...");
	UPackage* Package = CreateAssetPackage(MatPath, false);
	NewObject<UMaterialFactoryNew>()->FactoryCreateNew(UMaterial::StaticClass(), Package, FName(*MatName), RF_Public | RF_Standalone, NULL, GWarn);
	Material = Cast<UMaterial>(UUtuPluginLibrary::TryGetAsset(MatPath));
	LogAssetImportedOrFailed(Material, { MatDir, MatName, MatPath }, "", "Material", { });

	// Process Material
	if (Material != nullptr)
	{
		UTU_LOG_L("        Building Material...");
		Material->PreEditChange(NULL);
		Material->GetOutermost()->FullyLoad();
		Material->MarkPackageDirty();

		int H = -2000;
		int V = 700;
		// Settings
		GetOrCreateScalarParameter(Material, ImportSettings.MetallicIntensityMultiplier, "Utu_MetallicIntensityMultiplier", H, V, InUtuMaterial);
		H += 300;
		GetOrCreateScalarParameter(Material, ImportSettings.SpecularIntensityMultiplier, "Utu_SpecularIntensityMultiplier", H, V, InUtuMaterial);
		H += 300;
		GetOrCreateScalarParameter(Material, ImportSettings.EmissiveIntensityMultiplier, "Utu_EmissiveIntensityMultiplier", H, V, InUtuMaterial);
		H += 300;
		GetOrCreateScalarParameter(Material, ImportSettings.NormalIntensityMultiplier, "Utu_NormalIntensityMultiplier", H, V, InUtuMaterial);
		H += 300;
		GetOrCreateScalarParameter(Material, ImportSettings.OcclusionIntensityMultiplier, "Utu_OcclusionIntensityMultiplier", H, V, InUtuMaterial);
		H += 300;
		GetOrCreateScalarParameter(Material, ImportSettings.TexturesPannerTime, "Utu_TexturesPannerTime", H, V, InUtuMaterial);


		H = -2000;
		V = 1000;
		// Floats
		for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_floats_names.Num(), InUtuMaterial.material_floats.Num()); Idx++)
		{
			FString Name = InUtuMaterial.material_floats_names[Idx];
			float Value = InUtuMaterial.material_floats[Idx];
			UMaterialExpressionScalarParameter* Scalar = GetOrCreateScalarParameter(Material, Value, *Name, H, V, InUtuMaterial);
			H += 300;
		}

		H = -2000;
		V = 1300;
		// Textures & Panners
		for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_textures_names.Num(), InUtuMaterial.material_textures.Num()); Idx++)
		{
			FString Name = InUtuMaterial.material_textures_names[Idx];
			UTexture2D* Value = GetTextureFromUnityRelativeFilename(InUtuMaterial.material_textures[Idx]);
			UMaterialExpressionTextureSampleParameter2D* Texture = GetOrCreateTextureParameter(Material, Value, *Name, H + 600, V, InUtuMaterial);

			int TexCoordIndex = InUtuMaterial.material_vector2s_names.Find(Name + "_ST_TexCoord");
			int PannerIndex = InUtuMaterial.material_vector2s_names.Find(Name + "_ST_Panner");
			if (TexCoordIndex >= 0 && PannerIndex >= 0)
			{
				FString TexCoordName = InUtuMaterial.material_vector2s_names[TexCoordIndex];
				FVector2D TexCoordValue = InUtuMaterial.material_vector2s[TexCoordIndex];
				UMaterialExpressionTextureCoordinate* TexCoordNode = GetOrCreateTexCoordExpression(Material, FVector2D(1.0f, 1.0f), TexCoordName + "_Node", H, V, InUtuMaterial);
				UMaterialExpressionVectorParameter* TexCoord = GetOrCreateVectorParameter(Material, FLinearColor(TexCoordValue.X, TexCoordValue.Y, 0.0f, 0.0f), *TexCoordName, H, V + 50, InUtuMaterial);
				UMaterialExpressionComponentMask* TexCoordMask = GetOrCreateMaskExpression(Material, TexCoordName + "_Mask", H + 200, V + 50, InUtuMaterial);
				UMaterialExpressionMultiply* TexCoordMultiply = GetOrCreateMultiplyExpression(Material, TexCoordName + "_Multiply", H + 350, V, InUtuMaterial);

				FString PannerName = InUtuMaterial.material_vector2s_names[PannerIndex];
				FVector2D PannerValue = InUtuMaterial.material_vector2s[PannerIndex];
				UMaterialExpressionPanner* Panner = GetOrCreatePannerExpression(Material, FVector2D(0.0f, 0.0f), PannerName + "_Node", H + 475, V, InUtuMaterial);
				UMaterialExpressionScalarParameter* PannerTime = GetOrCreateScalarParameter(Material, ImportSettings.TexturesPannerTime, *("Utu_TexturesPannerTime_" + Name), H, V + 250, InUtuMaterial);

				if (TexCoordNode != nullptr && TexCoord != nullptr && TexCoordMask != nullptr && TexCoordMultiply != nullptr && PannerTime != nullptr && Panner != nullptr)
				{
					TexCoordNode->ConnectExpression(&TexCoordMultiply->A, 0);
					TexCoord->ConnectExpression(&TexCoordMask->Input, 2);
					TexCoordMask->ConnectExpression(&TexCoordMultiply->B, 0);
					TexCoordMultiply->ConnectExpression(&Panner->Coordinate, 0);
					PannerTime->ConnectExpression(&Panner->Time, 0);
					Panner->ConnectExpression(&Texture->Coordinates, 0);
				}
			}
			H += 900;
		}


		H = -2000;
		V = 1800;
		// Colors
		for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_colors_names.Num(), InUtuMaterial.material_colors.Num()); Idx++)
		{
			FString Name = InUtuMaterial.material_colors_names[Idx];
			FLinearColor Value = HexToColor(InUtuMaterial.material_colors[Idx]);
			UMaterialExpressionVectorParameter* Color = GetOrCreateVectorParameter(Material, Value, *Name, H, V, InUtuMaterial);
			H += 300;
		}


		H = -2000;
		V = 2100;
		// Vectors
		for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_vectors_names.Num(), InUtuMaterial.material_vectors.Num()); Idx++)
		{
			FString Name = InUtuMaterial.material_vectors_names[Idx];
			FQuat Value = InUtuMaterial.material_vectors[Idx];
			UMaterialExpressionVectorParameter* Vector = GetOrCreateVectorParameter(Material, FLinearColor(Value.X, Value.Y, Value.Z, Value.W), *Name, H, V, InUtuMaterial);
			H += 300;
		}


		H = -2000;
		V = 2400;
		// Vector2s
		for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_vector2s_names.Num(), InUtuMaterial.material_vector2s.Num()); Idx++)
		{
			FString Name = InUtuMaterial.material_vector2s_names[Idx];
			FVector2D Value = InUtuMaterial.material_vector2s[Idx];
			if (!Name.EndsWith("_ST_TexCoord") && !Name.EndsWith("_ST_Panner"))
			{
				UMaterialExpressionVectorParameter* Vector = GetOrCreateVectorParameter(Material, FLinearColor(Value.X, Value.Y, 0.0f, 0.0f), *Name, H, V, InUtuMaterial);
				H += 300;
			}
		}


		H = -2000;
		V = 2700;
		// Ints
		for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_ints_names.Num(), InUtuMaterial.material_ints.Num()); Idx++)
		{
			FString Name = InUtuMaterial.material_ints_names[Idx];
			int Value = InUtuMaterial.material_ints[Idx];
			UMaterialExpressionScalarParameter* Scalar = GetOrCreateScalarParameter(Material, Value, *Name, H, V, InUtuMaterial);
			H += 300;
		}


		// Custom material visual
		H = -600;
		V = 0;
		{
			// Default blend mode
			Material->BlendMode = EBlendMode::BLEND_Masked;
			// Default nodes
			UTexture2D* MainTextureAsset = GetTextureFromUnityRelativeFilename(InUtuMaterial.main_texture);
			if (MainTextureAsset == nullptr)
			{
				MainTextureAsset = Cast<UTexture2D>(UUtuPluginLibrary::TryGetAsset("/UtuPlugin/Default/Texture"));
			}
			UMaterialExpressionTextureSampleParameter2D* MainTexture = GetOrCreateTextureParameter(Material, MainTextureAsset, "__Main_Texture", H, V, InUtuMaterial);
			UMaterialExpressionPanner* MainTexturePanner = GetOrCreatePannerExpression(Material, InUtuMaterial.main_texture_offset, "__Main_Texture_panner", H - 150, V, InUtuMaterial);
			UMaterialExpressionTextureCoordinate* MainTextureTexCoord = GetOrCreateTexCoordExpression(Material, InUtuMaterial.main_texture_scale, "__Main_Texture_TexCoord", H - 350, V, InUtuMaterial);
			UMaterialExpressionScalarParameter* MainTexturePannerTime = GetOrCreateScalarParameter(Material, ImportSettings.TexturesPannerTime, "Utu_TexturesPannerTime___Main_Texture", H - 500, V + 100, InUtuMaterial);
			UMaterialExpressionVectorParameter* MainTextureColor = GetOrCreateVectorParameter(Material, HexToColor(InUtuMaterial.main_color), "__Main_Color", H, V + 250, InUtuMaterial);
			UMaterialExpressionMultiply* MainTextureMultiply = GetOrCreateMultiplyExpression(Material, "__Main_Texture_Multiply_With_Color", H + 300, V + 100, InUtuMaterial);
			UMaterialExpressionMultiply* MainTextureAlphaMultiply = GetOrCreateMultiplyExpression(Material, "__Main_Texture_Multiply_With_Color_Alpha", H + 300, V + 250, InUtuMaterial);
			if (MainTexture != nullptr && MainTextureColor != nullptr && MainTextureMultiply != nullptr && MainTextureAlphaMultiply != nullptr && MainTexturePanner != nullptr && MainTexturePannerTime != nullptr && MainTextureTexCoord != nullptr) 
			{
#if ENGINE_MAJOR_VERSION >= 5
				Material->GetEditorOnlyData()->Opacity.Connect(0, MainTextureAlphaMultiply);
				Material->GetEditorOnlyData()->OpacityMask.Connect(0, MainTextureAlphaMultiply);
				Material->GetEditorOnlyData()->BaseColor.Connect(0, MainTextureMultiply);
#else
				Material->Opacity.Connect(0, MainTextureAlphaMultiply);
				Material->OpacityMask.Connect(0, MainTextureAlphaMultiply);
				Material->BaseColor.Connect(0, MainTextureMultiply);
#endif
				MainTexture->ConnectExpression(&MainTextureMultiply->A, 0);
				MainTextureColor->ConnectExpression(&MainTextureMultiply->B, 0);
				MainTexture->ConnectExpression(&MainTextureAlphaMultiply->A, 4);
				MainTextureColor->ConnectExpression(&MainTextureAlphaMultiply->B, 4);
				MainTexturePanner->ConnectExpression(&MainTexture->Coordinates, 0);
				MainTexturePannerTime->ConnectExpression(&MainTexturePanner->Time, 0);
				MainTextureTexCoord->ConnectExpression(&MainTexturePanner->Coordinate, 0);
			}
		}
	}

	Material->PostEditChange();
	FGlobalComponentReregisterContext RecreateComponents;
	return Material;
}


FLinearColor FUtuPluginAssetTypeProcessor::HexToColor(FString InHex) {
	return FLinearColor(FColor::FromHex(InHex));
}


UTexture2D* FUtuPluginAssetTypeProcessor::GetTextureFromUnityRelativeFilename(FString InUnityRelativeFilename) {
	if (InUnityRelativeFilename != "") {
		TArray<FString> TexNames = FormatRelativeFilenameForUnreal(InUnityRelativeFilename);
		UTU_LOG_L("            Texture: " + TexNames[2]);
		UTexture2D* TextureAsset = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *TexNames[2]));
		if (TextureAsset == nullptr) {
			UTU_LOG_W("                Failed to associate texture because it doesn't exists: '" + TexNames[2] + "'");
		}
		return TextureAsset;
	}
	UTU_LOG_L("            Texture: None");
	return nullptr;
}


UMaterialExpressionTextureSampleParameter2D* FUtuPluginAssetTypeProcessor::GetOrCreateTextureParameter(UMaterial* InMaterial, UTexture* InTexture, FName InParamName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial) {
	UMaterialExpressionTextureSampleParameter2D* Ret = nullptr;
	UTU_LOG_L("                Texture Parameter: " + InParamName.ToString());
	if (InMaterial != nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
		for (UMaterialExpression* Exp : InMaterial->GetExpressions()) {
#else
		for (UMaterialExpression* Exp : InMaterial->Expressions) {
#endif
			UMaterialExpressionTextureSampleParameter2D* Parameter = Cast<UMaterialExpressionTextureSampleParameter2D>(Exp);
			if (Parameter != nullptr && Parameter->ParameterName == InParamName) {
				UTU_LOG_L("                    Texture Parameter found in material.");
				Ret = Parameter;
				break;
			}
		}
		if (Ret == nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
			Ret = Cast<UMaterialExpressionTextureSampleParameter2D>(UMaterialEditingLibrary::CreateMaterialExpression(InMaterial, UMaterialExpressionTextureSampleParameter2D::StaticClass()));
			if (Ret != nullptr) {
				UTU_LOG_L("                    Texture Parameter added into material.");
				Ret->ParameterName = InParamName;
				InMaterial->AddExpressionParameter(Ret, InMaterial->EditorParameters);
			}
			else {
				UTU_LOG_E("                    Failed to create Texture Parameter expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#else
			Ret = NewObject<UMaterialExpressionTextureSampleParameter2D>(InMaterial);
			if (Ret != nullptr) {
				UTU_LOG_L("                    Texture Parameter added into material.");
				Ret->ParameterName = InParamName;
				InMaterial->Expressions.Add(Ret);
			}
			else {
				UTU_LOG_E("                    Failed to create Texture Parameter expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#endif
		}
		if (Ret != nullptr) {
			Ret->Texture = InTexture;
			Ret->SamplerType = SAMPLERTYPE_Color;
			Ret->MaterialExpressionEditorX = InPosX;
			Ret->MaterialExpressionEditorY = InPosY;
		}
	}
	return Ret;
}

UMaterialExpressionScalarParameter* FUtuPluginAssetTypeProcessor::GetOrCreateScalarParameter(UMaterial* InMaterial, float InValue, FName InParamName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial) {
	UMaterialExpressionScalarParameter* Ret = nullptr;
	UTU_LOG_L("                Scalar Parameter: " + InParamName.ToString());
	if (InMaterial != nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
		for (UMaterialExpression* Exp : InMaterial->GetExpressions()) {
#else
		for (UMaterialExpression* Exp : InMaterial->Expressions) {
#endif
			UMaterialExpressionScalarParameter* Parameter = Cast<UMaterialExpressionScalarParameter>(Exp);
			if (Parameter != nullptr && Parameter->ParameterName == InParamName) {
				UTU_LOG_L("                    Scalar Parameter found in material.");
				Ret = Parameter;
				break;
			}
		}
		if (Ret == nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
			Ret = Cast<UMaterialExpressionScalarParameter>(UMaterialEditingLibrary::CreateMaterialExpression(InMaterial, UMaterialExpressionScalarParameter::StaticClass()));
			if (Ret != nullptr) {
				UTU_LOG_L("                    Scalar Parameter added into material.");
				Ret->ParameterName = InParamName;
				InMaterial->AddExpressionParameter(Ret, InMaterial->EditorParameters);
			}
			else {
				UTU_LOG_E("                    Failed to create Scalar Parameter expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#else
			Ret = NewObject<UMaterialExpressionScalarParameter>(InMaterial);
			if (Ret != nullptr) {
				UTU_LOG_L("                    Scalar Parameter added into material.");
				Ret->ParameterName = InParamName;
				InMaterial->Expressions.Add(Ret);
			}
			else {
				UTU_LOG_E("                    Failed to create Scalar Parameter expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#endif
		}
		if (Ret != nullptr) {
			Ret->DefaultValue = InValue;
			Ret->MaterialExpressionEditorX = InPosX;
			Ret->MaterialExpressionEditorY = InPosY;
		}
	}
	return Ret;
}

UMaterialExpressionVectorParameter* FUtuPluginAssetTypeProcessor::GetOrCreateVectorParameter(UMaterial* InMaterial, FLinearColor InColor, FName InParamName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial) {
	UMaterialExpressionVectorParameter* Ret = nullptr;
	UTU_LOG_L("                Vector Parameter: " + InParamName.ToString());
	if (InMaterial != nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
		for (UMaterialExpression* Exp : InMaterial->GetExpressions()) {
#else
		for (UMaterialExpression* Exp : InMaterial->Expressions) {
#endif
			UMaterialExpressionVectorParameter* Parameter = Cast<UMaterialExpressionVectorParameter>(Exp);
			if (Parameter != nullptr && Parameter->ParameterName == InParamName) {
				UTU_LOG_L("                    Vector Parameter found in material.");
				Ret = Parameter;
				break;
			}
		}
		if (Ret == nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
			Ret = Cast<UMaterialExpressionVectorParameter>(UMaterialEditingLibrary::CreateMaterialExpression(InMaterial, UMaterialExpressionVectorParameter::StaticClass()));
			if (Ret != nullptr) {
				UTU_LOG_L("                    Vector Parameter added into material.");
				Ret->ParameterName = InParamName;
				InMaterial->AddExpressionParameter(Ret, InMaterial->EditorParameters);
			}
			else {
				UTU_LOG_E("                    Failed to create Vector Parameter expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#else
			Ret = NewObject<UMaterialExpressionVectorParameter>(InMaterial);
			if (Ret != nullptr) {
				UTU_LOG_L("                    Vector Parameter added into material.");
				Ret->ParameterName = InParamName;
				InMaterial->Expressions.Add(Ret);
			}
			else {
				UTU_LOG_E("                    Failed to create Vector Parameter expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#endif
		}
		if (Ret != nullptr) {
			Ret->DefaultValue = InColor;
			Ret->MaterialExpressionEditorX = InPosX;
			Ret->MaterialExpressionEditorY = InPosY;
		}
	}
	return Ret;
}

UMaterialExpressionComponentMask* FUtuPluginAssetTypeProcessor::GetOrCreateMaskExpression(UMaterial* InMaterial, FString InExpressionName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial) {
	UMaterialExpressionComponentMask* Ret = nullptr;
	UTU_LOG_L("                Mask Expression: " + InExpressionName);
	if (InMaterial != nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
		for (UMaterialExpression* Exp : InMaterial->GetExpressions()) {
#else
		for (UMaterialExpression* Exp : InMaterial->Expressions) {
#endif
			UMaterialExpressionComponentMask* Expression = Cast<UMaterialExpressionComponentMask>(Exp);
			if (Expression != nullptr && Expression->Desc == InExpressionName) {
				UTU_LOG_L("                    Mask Expression found in material.");
				Ret = Expression;
				break;
			}
		}
		if (Ret == nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
			Ret = Cast<UMaterialExpressionComponentMask>(UMaterialEditingLibrary::CreateMaterialExpression(InMaterial, UMaterialExpressionComponentMask::StaticClass()));
			if (Ret != nullptr) {
				UTU_LOG_L("                   Mask Expression added into material.");
				Ret->Desc = InExpressionName;
				InMaterial->AddExpressionParameter(Ret, InMaterial->EditorParameters);
			}
			else {
				UTU_LOG_E("                    Failed to create Mask Expression expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#else
			Ret = NewObject<UMaterialExpressionComponentMask>(InMaterial);
			if (Ret != nullptr) {
				UTU_LOG_L("                   Mask Expression added into material.");
				Ret->Desc = InExpressionName;
				InMaterial->Expressions.Add(Ret);
			}
			else {
				UTU_LOG_E("                    Failed to create Mask Expression expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#endif
		}
		if (Ret != nullptr) {
			Ret->MaterialExpressionEditorX = InPosX;
			Ret->MaterialExpressionEditorY = InPosY;
			Ret->R = true;
			Ret->G = true;
			Ret->B = false;
			Ret->A = false;
		}
	}
	return Ret;
}

UMaterialExpressionMultiply* FUtuPluginAssetTypeProcessor::GetOrCreateMultiplyExpression(UMaterial* InMaterial, FString InExpressionName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial) {
	UMaterialExpressionMultiply* Ret = nullptr;
	UTU_LOG_L("                Multiply Expression: " + InExpressionName);
	if (InMaterial != nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
		for (UMaterialExpression* Exp : InMaterial->GetExpressions()) {
#else
		for (UMaterialExpression* Exp : InMaterial->Expressions) {
#endif
			UMaterialExpressionMultiply* Expression = Cast<UMaterialExpressionMultiply>(Exp);
			if (Expression != nullptr && Expression->Desc == InExpressionName) {
				UTU_LOG_L("                    Multiply Expression found in material.");
				Ret = Expression;
				break;
			}
		}
		if (Ret == nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
			Ret = Cast<UMaterialExpressionMultiply>(UMaterialEditingLibrary::CreateMaterialExpression(InMaterial, UMaterialExpressionMultiply::StaticClass()));
			if (Ret != nullptr) {
				UTU_LOG_L("                   Multiply Expression added into material.");
				Ret->Desc = InExpressionName;
				InMaterial->AddExpressionParameter(Ret, InMaterial->EditorParameters);
			}
			else {
				UTU_LOG_E("                    Failed to createMultiply Expression expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#else
			Ret = NewObject<UMaterialExpressionMultiply>(InMaterial);
			if (Ret != nullptr) {
				UTU_LOG_L("                   Multiply Expression added into material.");
				Ret->Desc = InExpressionName;
				InMaterial->Expressions.Add(Ret);
			}
			else {
				UTU_LOG_E("                    Failed to createMultiply Expression expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#endif
		}
		if (Ret != nullptr) {
			Ret->MaterialExpressionEditorX = InPosX;
			Ret->MaterialExpressionEditorY = InPosY;
		}
	}
	return Ret;
}

UMaterialExpressionPanner* FUtuPluginAssetTypeProcessor::GetOrCreatePannerExpression(UMaterial* InMaterial, FVector2D InValue, FString InExpressionName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial) {
	UMaterialExpressionPanner* Ret = nullptr;
	UTU_LOG_L("                Panner Expression: " + InExpressionName);
	if (InMaterial != nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
		for (UMaterialExpression* Exp : InMaterial->GetExpressions()) {
#else
		for (UMaterialExpression* Exp : InMaterial->Expressions) {
#endif
			UMaterialExpressionPanner* Expression = Cast<UMaterialExpressionPanner>(Exp);
			if (Expression != nullptr && Expression->Desc == InExpressionName) {
				UTU_LOG_L("                    Panner Expression found in material.");
				Ret = Expression;
				break;
			}
		}
		if (Ret == nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
			Ret = Cast<UMaterialExpressionPanner>(UMaterialEditingLibrary::CreateMaterialExpression(InMaterial, UMaterialExpressionPanner::StaticClass()));
			if (Ret != nullptr) {
				UTU_LOG_L("                   Panner Expression added into material.");
				Ret->Desc = InExpressionName;
				InMaterial->AddExpressionParameter(Ret, InMaterial->EditorParameters);
			}
			else {
				UTU_LOG_E("                    Failed to create Panner Expression expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#else
			Ret = NewObject<UMaterialExpressionPanner>(InMaterial);
			if (Ret != nullptr) {
				UTU_LOG_L("                   Panner Expression added into material.");
				Ret->Desc = InExpressionName;
				InMaterial->Expressions.Add(Ret);
			}
			else {
				UTU_LOG_E("                    Failed to create Panner Expression expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#endif
		}
		if (Ret != nullptr) {
			Ret->MaterialExpressionEditorX = InPosX;
			Ret->MaterialExpressionEditorY = InPosY;
			Ret->SpeedX = InValue.X;
			Ret->SpeedY = InValue.Y;
		}
	}
	return Ret;
}

UMaterialExpressionTextureCoordinate* FUtuPluginAssetTypeProcessor::GetOrCreateTexCoordExpression(UMaterial* InMaterial, FVector2D InValue, FString InExpressionName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial) {
	UMaterialExpressionTextureCoordinate* Ret = nullptr;
	UTU_LOG_L("                TexCoord Expression: " + InExpressionName);
	if (InMaterial != nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
		for (UMaterialExpression* Exp : InMaterial->GetExpressions()) {
#else
		for (UMaterialExpression* Exp : InMaterial->Expressions) {
#endif
			UMaterialExpressionTextureCoordinate* Expression = Cast<UMaterialExpressionTextureCoordinate>(Exp);
			if (Expression != nullptr && Expression->Desc == InExpressionName) {
				UTU_LOG_L("                    TexCoord Expression found in material.");
				Ret = Expression;
				break;
			}
		}
		if (Ret == nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
			Ret = Cast<UMaterialExpressionTextureCoordinate>(UMaterialEditingLibrary::CreateMaterialExpression(InMaterial, UMaterialExpressionTextureCoordinate::StaticClass()));
			if (Ret != nullptr) {
				UTU_LOG_L("                   TexCoord Expression added into material.");
				Ret->Desc = InExpressionName;
				InMaterial->AddExpressionParameter(Ret, InMaterial->EditorParameters);
			}
			else {
				UTU_LOG_E("                    Failed to create TexCoord Expression expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#else
			Ret = NewObject<UMaterialExpressionTextureCoordinate>(InMaterial);
			if (Ret != nullptr) {
				UTU_LOG_L("                   TexCoord Expression added into material.");
				Ret->Desc = InExpressionName;
				InMaterial->Expressions.Add(Ret);
			}
			else {
				UTU_LOG_E("                    Failed to create TexCoord Expression expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#endif
		}
		if (Ret != nullptr) {
			Ret->MaterialExpressionEditorX = InPosX;
			Ret->MaterialExpressionEditorY = InPosY;
			Ret->UTiling = InValue.X;
			Ret->VTiling = InValue.Y;
		}
	}
	return Ret;
}

void FUtuPluginAssetTypeProcessor::ProcessTexture(FUtuPluginTexture InUtuTexture) {
	// Format Paths
	TArray<FString> AssetNames = StartProcessAsset(InUtuTexture);
	// Invalid Asset
	if (DeleteInvalidAssetIfNeeded(AssetNames, UTexture2D::StaticClass())) {
		// Existing Asset
		UTexture2D* Asset = Cast<UTexture2D>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
		LogAssetImportOrReimport(Asset);
		// Create Asset
		AssetTools->Get().ImportAssetTasks({ BuildTask(InUtuTexture.texture_file_absolute_filename, AssetNames, nullptr) });
		Asset = Cast<UTexture2D>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
		if (Asset != nullptr) {
			Asset->PreEditChange(NULL);
			Asset->CompressionSettings = ImportSettings.CompressionSettings;
			Asset->Filter = ImportSettings.Filter;
			Asset->LODGroup = ImportSettings.LODGroup;
			Asset->SRGB = ImportSettings.SRGB;
			Asset->MaxTextureSize = ImportSettings.MaxTextureSize;
			Asset->CompressionQuality = ImportSettings.CompressionQuality;
			Asset->MipGenSettings = ImportSettings.MipGenSettings;
			Asset->DeferCompression = false;
			Asset->PostEditChange();
		}
		LogAssetImportedOrFailed(Asset, AssetNames, InUtuTexture.texture_file_absolute_filename, "Texture", { "Invalid Texture File : Make sure that the texture file is a supported format by trying to import it manually in Unreal." });
	}
}

void FUtuPluginAssetTypeProcessor::LogAssetCreateOrNot(UObject* InAsset) {
	if (InAsset == nullptr) {
		UTU_LOG_L("    New Asset. Creating...");
	}
	else {
		UTU_LOG_L("    Existing Asset.");
	}
}

void FUtuPluginAssetTypeProcessor::LogAssetImportOrReimport(UObject* InAsset) {
	if (InAsset == nullptr) {
		UTU_LOG_L("    New Asset. Importing..."); 
	}
	else { 
		UTU_LOG_L("    Existing Asset. Re-Importing..."); 
	}
}

void FUtuPluginAssetTypeProcessor::LogAssetImportedOrFailed(UObject* InAsset, TArray<FString> InAssetNames, FString InSourceFileFullname, FString InAssetType, TArray<FString> InPotentialCauses) {
	if (InAsset != nullptr) {
		FAssetRegistryModule::AssetCreated(InAsset);
		UTU_LOG_L("            Asset Created.");
	}
	else {
		UTU_LOG_E("    Asset Name: " + InAssetNames[1]);
		UTU_LOG_E("        Unreal Asset Relative Filename: " + InAssetNames[2]);
		if (InSourceFileFullname != "") {
			UTU_LOG_E("        Source File Fullname: " + InSourceFileFullname);
		}
		UTU_LOG_E("            Failed to create new " + InAssetType);
		UTU_LOG_E("            Potential Causes:");
		if (InPotentialCauses.Num() == 0) {
			InPotentialCauses.Add("No Potential Causes known yet.");
		}
		for (FString x : InPotentialCauses) {
			UTU_LOG_E("                - " + x);
		}
		UTU_LOG_E("            Asset skipped.");
	}
}


void FUtuPluginAssetTypeProcessor::ProcessPrefabFirstPass(FUtuPluginPrefabFirstPass InUtuPrefabFirstPass) {
	// Make sure it does not save the bp on compile
	UBlueprintEditorSettings* Settings = GetMutableDefault<UBlueprintEditorSettings>();
	ESaveOnCompile OriginalSaveOnCompile = Settings->SaveOnCompile;
	Settings->SaveOnCompile = ESaveOnCompile::SoC_Never;
	Settings->SaveConfig();
	// Format Paths
	TArray<FString> AssetNames = StartProcessAsset(InUtuPrefabFirstPass);
	// Invalid Asset
	if (DeleteInvalidAssetIfNeeded(AssetNames, UBlueprint::StaticClass())) {
		// Existing Asset
		UBlueprint* Asset = Cast<UBlueprint>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
		LogAssetCreateOrNot(Asset);
		// Create Asset
		IKismetCompilerInterface& KismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
		if (Asset == nullptr) {
			// Create Bp
			UPackage* Package = CreateAssetPackage(AssetNames[2], false);
			UClass* BlueprintClass = nullptr;
			UClass* BlueprintGeneratedClass = nullptr;
			KismetCompilerModule.GetBlueprintTypesForClass(AActor::StaticClass(), BlueprintClass, BlueprintGeneratedClass);
			Asset = FKismetEditorUtilities::CreateBlueprint(AActor::StaticClass(), Package, *AssetNames[1], BPTYPE_Normal, BlueprintClass, BlueprintGeneratedClass, FName("LevelEditorActions"));
			LogAssetImportedOrFailed(Asset, AssetNames, "", "Blueprint", { });
			// Create Root Node
			BpAddRootComponent(Asset, InUtuPrefabFirstPass.has_any_static_child);
		}
		if (Asset != nullptr) {
			Asset->PreEditChange(NULL);
			// Delete Existing Nodes -- TODO : Analyse them and keep the good ones
			TArray<USCS_Node*> ExistingNodes = Asset->SimpleConstructionScript->GetAllNodes();
			if (ExistingNodes.Num() > 1) {
				UTU_LOG_L("    Detected existing Nodes. Deleting them and re-building the Blueprint from scratch.");
				//UTU_LOG_L("        Plan for a future release: Analyse existing Nodes and keep the good ones.");
				//for (USCS_Node* Node : ExistingNodes) {
				//	Node->EditorComponentInstance->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
				//}
				while (ExistingNodes.Num() > 1) {
					TArray<USCS_Node*> RootNodes = Asset->SimpleConstructionScript->GetRootNodes();
					for (USCS_Node* Node : ExistingNodes) {
						if (!RootNodes.Contains(Node)) {
							Asset->SimpleConstructionScript->RemoveNode(Node);
						}
					}
					Asset->SimpleConstructionScript->FixupRootNodeParentReferences();
					ExistingNodes = Asset->SimpleConstructionScript->GetAllNodes();
				}
			}
			Cast<USceneComponent>(ExistingNodes[0]->ComponentTemplate)->SetMobility(InUtuPrefabFirstPass.has_any_static_child ? EComponentMobility::Static : EComponentMobility::Movable);
			FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Asset);
			FKismetEditorUtilities::CompileBlueprint(Asset);
			Asset->PostEditChange();
		}
	}
	// Restore Save On Compile
	Settings->SaveOnCompile = OriginalSaveOnCompile;
	Settings->SaveConfig();
}

void FUtuPluginAssetTypeProcessor::ProcessPrefabSecondPass(FUtuPluginPrefabSecondPass InUtuPrefabSecondPass) {
	// Make sure it does not save the bp on compile
	UBlueprintEditorSettings* Settings = GetMutableDefault<UBlueprintEditorSettings>();
	ESaveOnCompile OriginalSaveOnCompile = Settings->SaveOnCompile;
	Settings->SaveOnCompile = ESaveOnCompile::SoC_Never;
	Settings->SaveConfig();
	// Format Paths
	TArray<FString> AssetNames = StartProcessAsset(InUtuPrefabSecondPass);
	// Existing Asset
	UBlueprint* Asset = Cast<UBlueprint>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
	// Skipping Asset. Should already be created by FirstPass
	if (Asset == nullptr) {
		UTU_LOG_W("    Asset Name: " + AssetNames[1]);
		UTU_LOG_W("        Unreal Asset Relative Path: " + AssetNames[2]);
		UTU_LOG_W("            Cannot setup Blueprint because the asset does not exist.");
		UTU_LOG_W("            Potential Causes:");
		UTU_LOG_W("                - PrefabFirstPass failed to create the asset.");
		UTU_LOG_W("            Asset skipped.");
		return;
	}
	Asset->PreEditChange(NULL);
	// Process Asset
	// Maps
	TMap<int, USCS_Node*> IdToNode;
	TMap<USCS_Node*, int> NodeToParentId;
	// Get Root Node
	int RootId = UtuConst::INVALID_INT;
	USCS_Node* RootNode = Asset->SimpleConstructionScript->GetRootNodes()[0];
	IdToNode.Add(RootId, RootNode);
	// Add Real Components
	TArray<FString> UniqueNames;
	UTU_LOG_L("    Adding real components...");
	for (FUtuPluginActor PrefabComponent : InUtuPrefabSecondPass.prefab_components) {
		// New Component
		FString ComponentName = BpMakeUniqueName(PrefabComponent.actor_display_name, UniqueNames);
		USCS_Node* ComponentNode = nullptr;
		bool bComponentCreated = BpAddRootComponentForSubComponentsIfNeeded(Asset, PrefabComponent, ComponentName, ComponentNode);
		if (bComponentCreated) {
			IdToNode.Add(PrefabComponent.actor_id, ComponentNode);
			NodeToParentId.Add(ComponentNode, PrefabComponent.actor_parent_id);
		}
		// Real Components for real this time
		for (EUtuActorType CompType : PrefabComponent.actor_types) {
			USCS_Node* SubComponentNode = nullptr;
			FString SubComponentName = bComponentCreated ? BpMakeUniqueName(PrefabComponent.actor_display_name, UniqueNames) : ComponentName;
			if (CompType == EUtuActorType::Empty) {
				BpAddEmptyComponent(Asset, PrefabComponent, SubComponentName, SubComponentNode, bComponentCreated);
			}
			else if (CompType == EUtuActorType::StaticMesh) {
				BpAddStaticMeshComponent(Asset, PrefabComponent, SubComponentName, SubComponentNode, bComponentCreated);
			}
			else if (CompType == EUtuActorType::SkeletalMesh) {
				BpAddSkeletalMeshComponent(Asset, PrefabComponent, SubComponentName, SubComponentNode, bComponentCreated);  // TODO : Support Skeletal Mesh
				//BpAddStaticMeshComponent(Asset, PrefabComponent, SubComponentName, SubComponentNode, bComponentCreated);
			}
			else if (CompType == EUtuActorType::PointLight) {
				BpAddPointLightComponent(Asset, PrefabComponent, SubComponentName, SubComponentNode, bComponentCreated);
			}
			else if (CompType == EUtuActorType::DirectionalLight) {
				BpAddDirectionalLightComponent(Asset, PrefabComponent, SubComponentName, SubComponentNode, bComponentCreated);
			}
			else if (CompType == EUtuActorType::SpotLight) {
				BpAddSpotLightComponent(Asset, PrefabComponent, SubComponentName, SubComponentNode, bComponentCreated);
			}
			else if (CompType == EUtuActorType::Camera) {
				BpAddCameraComponent(Asset, PrefabComponent, SubComponentName, SubComponentNode, bComponentCreated);
			}
			else if (CompType == EUtuActorType::Prefab) {
				BpAddChildActorComponent(Asset, PrefabComponent, SubComponentName, SubComponentNode, bComponentCreated);
			}
			// Attachment
			if (bComponentCreated) {
				ComponentNode->AddChildNode(SubComponentNode);
			}
			else {
				IdToNode.Add(PrefabComponent.actor_id, SubComponentNode);
				NodeToParentId.Add(SubComponentNode, PrefabComponent.actor_parent_id);
			}
		}
	}
	// Parent Nodes
	if (Asset != nullptr) {
		UTU_LOG_L("        Parenting components...");
		TArray<USCS_Node*> Keys;
		NodeToParentId.GetKeys(Keys);
		for (USCS_Node* Node : Keys) {
			int Id = NodeToParentId[Node];
			USCS_Node* ParentNode = IdToNode[Id];
			ParentNode->AddChildNode(Node);
			UTU_LOG_L("            " + Node->GetName() + " -> " + ParentNode->GetName());
		}
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Asset);
		FKismetEditorUtilities::CompileBlueprint(Asset);
		Asset->MarkPackageDirty();
		Asset->PostEditChange();
	}
	// Restore Save On Compile
	Settings->SaveOnCompile = OriginalSaveOnCompile;
	Settings->SaveConfig();
	Settings->RemoveFromRoot();
}

void FUtuPluginAssetTypeProcessor::BpAddRootComponent(UBlueprint* InAsset, bool bStatic) {
	int RootNodeId = UtuConst::INVALID_INT;
	UTU_LOG_L("    Adding Root component...");
	UTU_LOG_L("        Component Name: 'Root'");
	UTU_LOG_L("        Component ID: " + FString::FromInt(RootNodeId));
	UTU_LOG_L("        Component Class: 'USceneComponent'");
	USceneComponent* Root = NewObject<USceneComponent>(InAsset, TEXT("Root"));
	Root->SetMobility(bStatic ? EComponentMobility::Static : EComponentMobility::Movable);
	Root->ComponentTags.Add(*FString::FromInt(RootNodeId));
	USCS_Node* RootNode = InAsset->SimpleConstructionScript->CreateNode(Root->GetClass(), Root->GetFName());
	//InAsset->SimpleConstructionScript->RemoveNode(InAsset->SimpleConstructionScript->GetRootNodes()[0]);
	InAsset->SimpleConstructionScript->AddNode(RootNode); // Making it the new root
	//USCS_Node* RootNode = InAsset->SimpleConstructionScript->GetRootNodes()[0];
	UEditorEngine::CopyPropertiesForUnrelatedObjects(Root, RootNode->ComponentTemplate);
	//FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(InAsset);
	// TODO : Handle Root Node! Can't Remove it because it's the root
	//OutRootNode = InAsset->SimpleConstructionScript->CreateNode(OutRoot->GetClass(), OutRoot->GetFName());
	//UEditorEngine::CopyPropertiesForUnrelatedObjects(OutRoot, OutRootNode->ComponentTemplate);
	//InAsset->SimpleConstructionScript->AddNode(RootNode); // Making it the new root
	//InAsset->SimpleConstructionScript->FixupRootNodeParentReferences();
}



AActor* FUtuPluginAssetTypeProcessor::WorldAddRootActorForSubActorsIfNeeded(UWorld* InAsset, FUtuPluginActor InUtuActor) {
	AActor* RetActor = nullptr;
	UTU_LOG_L("        Processing '" + InUtuActor.actor_display_name + "'...");
	if (InUtuActor.actor_types.Num() != 1 || InUtuActor.actor_types[0] == EUtuActorType::Empty) { // If actor_types == 1, we don't need to have an empty root above the other actors.
		UTU_LOG_L("            Because there was more than one supported components on this GameObject in Unity, creating another 'Root' to hold them...");
		UTU_LOG_L("                Adding Root actor...");
		UTU_LOG_L("                    Actor Name: '" + InUtuActor.actor_display_name + "'");
		UTU_LOG_L("                    Actor ID: " + FString::FromInt(InUtuActor.actor_id));
		UTU_LOG_L("                    Actor Class: 'AActor'");
		UTU_LOG_L("                    Actor Tag: '" + InUtuActor.actor_tag + "'");
		// Create Empty Root Actor
		FActorSpawnParameters Params = FActorSpawnParameters();
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		RetActor = InAsset->SpawnActor<AActor>(Params);
		if (RetActor != nullptr) {
			USceneComponent* RootComponent = NewObject<USceneComponent>(RetActor, USceneComponent::GetDefaultSceneRootVariableName(), RF_Transactional);
			if (RootComponent != nullptr) {
				RetActor->SetRootComponent(RootComponent);
				RetActor->AddInstanceComponent(RootComponent);
				RootComponent->RegisterComponent();
				RetActor->SetActorLabel(InUtuActor.actor_display_name);
				RetActor->Tags.Add(*FString::FromInt(InUtuActor.actor_id));
				if (InUtuActor.actor_tag != "Untagged") {
					RetActor->Tags.Add(*InUtuActor.actor_tag);
				}
				RetActor->SetActorHiddenInGame(!InUtuActor.actor_is_visible);
				RetActor->SetActorLocation(UtuConst::ConvertLocation(InUtuActor.actor_world_location));
				RetActor->SetActorRotation(UtuConst::ConvertRotation(InUtuActor.actor_world_rotation));
				RetActor->SetActorScale3D(UtuConst::ConvertScale(InUtuActor.actor_world_scale));
				RetActor->GetRootComponent()->SetMobility(InUtuActor.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
			}
			else {
				UTU_LOG_E("            Failed to spawn Root Actor's Root Component...");
				UTU_LOG_E("                Actor Name: '" + InUtuActor.actor_display_name + "'");
				UTU_LOG_E("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
				UTU_LOG_E("                Potential Causes:");
				UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
				return nullptr; // Don't even bother returning the actor
			}
		}
		else {
			UTU_LOG_E("            Failed to spawn Root Actor...");
			UTU_LOG_E("                Actor Name: '" + InUtuActor.actor_display_name + "'");
			UTU_LOG_E("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
			UTU_LOG_E("                Potential Causes:");
			UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
		}
	}
	return RetActor;
}

AActor* FUtuPluginAssetTypeProcessor::WorldSpawnStaticMeshActor(UWorld* InAsset, FUtuPluginActor InUtuActor) {
	UTU_LOG_L("            Adding Static Mesh Actor...");
	UTU_LOG_L("                Actor Name: '" + InUtuActor.actor_display_name + "'");
	UTU_LOG_L("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
	UTU_LOG_L("                Actor Class: 'AStaticMeshActor'");
	UTU_LOG_L("                Actor Tag: '" + InUtuActor.actor_tag + "'");
	FActorSpawnParameters Params = FActorSpawnParameters();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AStaticMeshActor* RetActor = InAsset->SpawnActor<AStaticMeshActor>(Params);
	if (RetActor != nullptr) {
		TArray<FString> MeshNames = FormatRelativeFilenameForUnreal(InUtuActor.actor_mesh.actor_mesh_relative_filename);
		TArray<FString> MeshNamesSeparated = FormatRelativeFilenameForUnreal(InUtuActor.actor_mesh.actor_mesh_relative_filename_if_separated);
		UTU_LOG_L("            Associating Static Mesh to Static Mesh Actor...");
		UTU_LOG_L("                Unreal Asset Relative Path: " + MeshNames[2]);
		UTU_LOG_L("                Unreal Asset Relative Path If Separated: " + MeshNamesSeparated[2]);
		UStaticMesh* StaticMeshAsset = Cast<UStaticMesh>(UUtuPluginLibrary::TryGetAsset(MeshNames[2]));
		if (StaticMeshAsset == nullptr && ImportSettings.bImportSeparated) {
			// Try separated way
			StaticMeshAsset = Cast<UStaticMesh>(UUtuPluginLibrary::TryGetAsset(MeshNamesSeparated[2]));
		}
		if (StaticMeshAsset == nullptr) {
			UTU_LOG_W("                Failed to assign Static Mesh because it doesn't exists: '" + MeshNames[2] + "'");
		}
		RetActor->GetStaticMeshComponent()->SetStaticMesh(StaticMeshAsset);

		UTU_LOG_L("            Associating Materials to Static Mesh...");
		for (int x = 0; x < InUtuActor.actor_mesh.actor_mesh_materials_relative_filenames.Num(); x++) {
			TArray<FString> MatNames = FormatRelativeFilenameForUnreal(InUtuActor.actor_mesh.actor_mesh_materials_relative_filenames[x], true);
			UTU_LOG_L("                MaterialId[" + FString::FromInt(x) + "] : " + MatNames[2]);
			UMaterialInterface* MaterialAsset = Cast<UMaterialInterface>(UUtuPluginLibrary::TryGetAsset(MatNames[2]));
			RetActor->GetStaticMeshComponent()->SetMaterial(x, MaterialAsset);
			if (MaterialAsset == nullptr) {
				UTU_LOG_W("                    Failed to assign material because it doesn't exists: '" + MatNames[2] + "'");
			}
		}
		RetActor->SetActorLabel(InUtuActor.actor_display_name);
	}
	else {
		UTU_LOG_E("            Failed to spawn StaticMesh Actor...");
		UTU_LOG_E("                Actor Name: '" + InUtuActor.actor_display_name + "'");
		UTU_LOG_E("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
		UTU_LOG_E("                Potential Causes:");
		UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
	}
	return RetActor;
}

AActor* FUtuPluginAssetTypeProcessor::WorldSpawnSkeletalMeshActor(UWorld* InAsset, FUtuPluginActor InUtuActor) {
	UTU_LOG_L("            Adding Skeletal Mesh Actor...");
	UTU_LOG_L("                Actor Name: '" + InUtuActor.actor_display_name + "'");
	UTU_LOG_L("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
	UTU_LOG_L("                Actor Class: 'ASkeletalMeshActor'");
	UTU_LOG_L("                Actor Tag: '" + InUtuActor.actor_tag + "'");
	FActorSpawnParameters Params = FActorSpawnParameters();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ASkeletalMeshActor* RetActor = InAsset->SpawnActor<ASkeletalMeshActor>(Params);
	if (RetActor != nullptr) {
		TArray<FString> MeshNames = FormatRelativeFilenameForUnreal(InUtuActor.actor_mesh.actor_mesh_relative_filename);
		UTU_LOG_L("            Associating Skeletal Mesh to Skeletal Mesh Actor...");
		UTU_LOG_L("                Unreal Asset Relative Path: " + MeshNames[2]);
		USkeletalMesh* SkeletalMeshAsset = Cast<USkeletalMesh>(UUtuPluginLibrary::TryGetAsset(MeshNames[2]));
		RetActor->GetSkeletalMeshComponent()->SetSkeletalMesh(SkeletalMeshAsset);
		if (SkeletalMeshAsset == nullptr) {
			UTU_LOG_W("                Failed to assign Skeletal Mesh because it doesn't exists: '" + MeshNames[2] + "'");
		}
		UTU_LOG_L("            Associating Materials to Skeletal Mesh...");
		for (int x = 0; x < InUtuActor.actor_mesh.actor_mesh_materials_relative_filenames.Num(); x++) {
			TArray<FString> MatNames = FormatRelativeFilenameForUnreal(InUtuActor.actor_mesh.actor_mesh_materials_relative_filenames[x], true);
			UTU_LOG_L("                MaterialId[" + FString::FromInt(x) + "] : " + MatNames[2]);
			UMaterialInterface* MaterialAsset = Cast<UMaterialInterface>(UUtuPluginLibrary::TryGetAsset(MatNames[2]));
			RetActor->GetSkeletalMeshComponent()->SetMaterial(x, MaterialAsset);
			if (MaterialAsset == nullptr) {
				UTU_LOG_W("                    Failed to assign material because it doesn't exists: '" + MatNames[2] + "'");
			}
		}
		RetActor->SetActorLabel(InUtuActor.actor_display_name);
	}
	else {
		UTU_LOG_E("            Failed to spawn StaticMesh Actor...");
		UTU_LOG_E("                Actor Name: '" + InUtuActor.actor_display_name + "'");
		UTU_LOG_E("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
		UTU_LOG_E("                Potential Causes:");
		UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
	}
	return RetActor;
}

AActor* FUtuPluginAssetTypeProcessor::WorldSpawnBlueprintActor(UWorld* InAsset, FUtuPluginActor InUtuActor) {
	TArray<FString> BpNames = FormatRelativeFilenameForUnreal(InUtuActor.actor_prefab.actor_prefab_relative_filename);
	UTU_LOG_L("            Adding Blueprint Actor...");
	UTU_LOG_L("                Actor Name: '" + InUtuActor.actor_display_name + "'");
	UTU_LOG_L("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
	UTU_LOG_L("                Actor Class: 'AActor'");
	UTU_LOG_L("                Actor Tag: '" + InUtuActor.actor_tag + "'");
	UTU_LOG_L("                Unreal Asset Relative Path: " + BpNames[2]);
	AActor* RetActor = nullptr;
	UBlueprint* BlueprintAsset = Cast<UBlueprint>(UUtuPluginLibrary::TryGetAsset(BpNames[2]));
	if (BlueprintAsset != nullptr) {
		FActorSpawnParameters Params = FActorSpawnParameters();
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		RetActor = InAsset->SpawnActor<AActor>(BlueprintAsset->GeneratedClass, Params);
		if (RetActor != nullptr) {
			RetActor->SetActorLabel(InUtuActor.actor_display_name);
		}
		else {
			UTU_LOG_E("            Failed to spawn Blueprint Actor...");
			UTU_LOG_E("                Actor Name: '" + InUtuActor.actor_display_name + "'");
			UTU_LOG_E("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
			UTU_LOG_E("                Unreal Asset Relative Path: " + BpNames[2]);
			UTU_LOG_E("                Potential Causes:");
			UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
		}
	}
	else {
		UTU_LOG_W("            Cannot spawn Blueprint Actor because Blueprint asset does not exist...");
		UTU_LOG_W("                Actor Name: '" + InUtuActor.actor_display_name + "'");
		UTU_LOG_W("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
		UTU_LOG_W("                Unreal Asset Relative Path: " + BpNames[2]);
	}
	return RetActor;
}

AActor* FUtuPluginAssetTypeProcessor::WorldSpawnSkyLightActor(UWorld* InAsset) {
	UTU_LOG_L("            Adding Point Sky Actor...");
	UTU_LOG_L("                Actor Name: 'SkyLight'");
	UTU_LOG_L("                Actor Class: 'ASkyLight'");
	FActorSpawnParameters Params = FActorSpawnParameters();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ASkyLight* RetActor = InAsset->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), Params);
	if (RetActor != nullptr && Cast<USkyLightComponent>(RetActor->GetLightComponent()) != nullptr) {
		USkyLightComponent* Comp = Cast<USkyLightComponent>(RetActor->GetLightComponent());
		RetActor->Tags.Add("UtuActor");
		RetActor->SetActorLabel("SkyLight");
		RetActor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
		Comp->Intensity = ImportSettings.SkyLightIntensity;
		Comp->SourceType = ESkyLightSourceType::SLS_SpecifiedCubemap;
		Comp->Cubemap = Cast<UTextureCube>(UUtuPluginLibrary::TryGetAsset("/UtuPlugin/Components/TX_CubeMap"));
	}
	else {
		UTU_LOG_E("            Failed to spawn Point Light Actor...");
		UTU_LOG_E("                Actor Name: 'SkyLight'");
		UTU_LOG_E("                Potential Causes:");
		UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
	}
	return RetActor;
}


AActor* FUtuPluginAssetTypeProcessor::WorldSpawnPointLightActor(UWorld* InAsset, FUtuPluginActor InUtuActor) {
	UTU_LOG_L("            Adding Point Light Actor...");
	UTU_LOG_L("                Actor Name: '" + InUtuActor.actor_display_name + "'");
	UTU_LOG_L("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
	UTU_LOG_L("                Actor Class: 'APointLight'");
	UTU_LOG_L("                Actor Tag: '" + InUtuActor.actor_tag + "'");
	FActorSpawnParameters Params = FActorSpawnParameters();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APointLight* RetActor = InAsset->SpawnActor<APointLight>(APointLight::StaticClass(), Params);
	if (RetActor != nullptr && Cast<UPointLightComponent>(RetActor->GetLightComponent()) != nullptr) {
		UPointLightComponent* Comp = Cast<UPointLightComponent>(RetActor->GetLightComponent());
		RetActor->SetActorLabel(InUtuActor.actor_display_name);
		Comp->Intensity = InUtuActor.actor_light.light_intensity * ImportSettings.LightIntensityMultiplier;
		Comp->bUseInverseSquaredFalloff = false;
		Comp->SetLightFalloffExponent(ImportSettings.LightFalloffExponent);
		Comp->SetLightColor(HexToColor(InUtuActor.actor_light.light_color));
		Comp->AttenuationRadius = InUtuActor.actor_light.light_range * ImportSettings.LightRangeMultiplier;
		Comp->SetAttenuationRadius(InUtuActor.actor_light.light_range * ImportSettings.LightRangeMultiplier);
		Comp->CastShadows = InUtuActor.actor_light.light_is_casting_shadows;
	}
	else {
		UTU_LOG_E("            Failed to spawn Point Light Actor...");
		UTU_LOG_E("                Actor Name: '" + InUtuActor.actor_display_name + "'");
		UTU_LOG_E("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
		UTU_LOG_E("                Potential Causes:");
		UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
	}
	return RetActor;
}


AActor* FUtuPluginAssetTypeProcessor::WorldSpawnDirectionalLightActor(UWorld* InAsset, FUtuPluginActor InUtuActor) {
	UTU_LOG_L("            Adding Directional Light Actor...");
	UTU_LOG_L("                Actor Name: '" + InUtuActor.actor_display_name + "'");
	UTU_LOG_L("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
	UTU_LOG_L("                Actor Class: 'ADirectionalLight'");
	UTU_LOG_L("                Actor Tag: '" + InUtuActor.actor_tag + "'");
	FActorSpawnParameters Params = FActorSpawnParameters();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ADirectionalLight* RetActor = InAsset->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), Params);
	if (RetActor != nullptr && RetActor->GetLightComponent() != nullptr) {
		ULightComponent* Comp = RetActor->GetLightComponent();
		RetActor->SetActorLabel(InUtuActor.actor_display_name);
		Comp->Intensity = InUtuActor.actor_light.light_intensity * ImportSettings.LightIntensityMultiplier;
		Comp->SetLightColor(HexToColor(InUtuActor.actor_light.light_color));
		Comp->CastShadows = InUtuActor.actor_light.light_is_casting_shadows;
	}
	else {
		UTU_LOG_E("            Failed to spawn Directional Light Actor...");
		UTU_LOG_E("                Actor Name: '" + InUtuActor.actor_display_name + "'");
		UTU_LOG_E("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
		UTU_LOG_E("                Potential Causes:");
		UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
	}
	return RetActor;
}

AActor* FUtuPluginAssetTypeProcessor::WorldSpawnSpotLightActor(UWorld* InAsset, FUtuPluginActor InUtuActor) {
	UTU_LOG_L("            Adding Spot Light Actor...");
	UTU_LOG_L("                Actor Name: '" + InUtuActor.actor_display_name + "'");
	UTU_LOG_L("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
	UTU_LOG_L("                Actor Class: 'ASpotLight'");
	UTU_LOG_L("                Actor Tag: '" + InUtuActor.actor_tag + "'");
	FActorSpawnParameters Params = FActorSpawnParameters();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ASpotLight* RetActor = InAsset->SpawnActor<ASpotLight>(ASpotLight::StaticClass(), Params);
	if (RetActor != nullptr && Cast<USpotLightComponent>(RetActor->GetLightComponent()) != nullptr) {
		USpotLightComponent* Comp = Cast<USpotLightComponent>(RetActor->GetLightComponent());
		RetActor->SetActorLabel(InUtuActor.actor_display_name);
		Comp->Intensity = InUtuActor.actor_light.light_intensity * ImportSettings.LightIntensityMultiplier;
		Comp->bUseInverseSquaredFalloff = false;
		Comp->SetLightFalloffExponent(ImportSettings.LightFalloffExponent);
		Comp->SetLightColor(HexToColor(InUtuActor.actor_light.light_color));
		Comp->AttenuationRadius = InUtuActor.actor_light.light_range * ImportSettings.LightRangeMultiplier;
		Comp->SetAttenuationRadius(InUtuActor.actor_light.light_range * ImportSettings.LightRangeMultiplier);
		Comp->CastShadows = InUtuActor.actor_light.light_is_casting_shadows;
		Comp->InnerConeAngle = ImportSettings.LightSpotInnerConeAngle;
		Comp->OuterConeAngle = InUtuActor.actor_light.light_spot_angle * ImportSettings.LightSpotAngleMultiplier;
	}
	else {
		UTU_LOG_E("            Failed to spawn Spot Light Actor...");
		UTU_LOG_E("                Actor Name: '" + InUtuActor.actor_display_name + "'");
		UTU_LOG_E("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
		UTU_LOG_E("                Potential Causes:");
		UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
	}
	return RetActor;
}

AActor* FUtuPluginAssetTypeProcessor::WorldSpawnCameraActor(UWorld* InAsset, FUtuPluginActor InUtuActor) {
	AActor* RetActor = nullptr;
	if (InUtuActor.actor_camera.camera_is_physical) {
		UTU_LOG_L("            Adding Cine Camera Actor...");
		UTU_LOG_L("                Actor Name: '" + InUtuActor.actor_display_name + "'");
		UTU_LOG_L("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
		UTU_LOG_L("                Actor Class: 'ACineCameraActor'");
		UTU_LOG_L("                Actor Tag: '" + InUtuActor.actor_tag + "'");
		FActorSpawnParameters Params = FActorSpawnParameters();
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		RetActor = InAsset->SpawnActor<ACineCameraActor>(ACineCameraActor::StaticClass(), Params);
		if (RetActor != nullptr && Cast<ACineCameraActor>(RetActor)->GetCineCameraComponent() != nullptr) {
			UCineCameraComponent* Comp = Cast<ACineCameraActor>(RetActor)->GetCineCameraComponent();
			RetActor->SetActorLabel(InUtuActor.actor_display_name);
			//InUtuActor.actor_camera.camera_viewport_rect;
			Comp->OrthoNearClipPlane = InUtuActor.actor_camera.camera_near_clip_plane;
			Comp->OrthoFarClipPlane = InUtuActor.actor_camera.camera_far_clip_plane;
			Comp->AspectRatio = InUtuActor.actor_camera.camera_aspect_ratio;
			Comp->ProjectionMode = InUtuActor.actor_camera.camera_is_perspective ? ECameraProjectionMode::Perspective : ECameraProjectionMode::Orthographic;
			Comp->OrthoWidth = InUtuActor.actor_camera.camera_ortho_size;
			Comp->FieldOfView = InUtuActor.actor_camera.camera_persp_field_of_view;
			Comp->CurrentFocalLength = InUtuActor.actor_camera.camera_phys_focal_length;
#if ENGINE_MINOR_VERSION >= 24 || ENGINE_MAJOR_VERSION >= 5
			Comp->Filmback.SensorWidth = InUtuActor.actor_camera.camera_phys_sensor_size.X;
			Comp->Filmback.SensorHeight = InUtuActor.actor_camera.camera_phys_sensor_size.Y;
			Comp->Filmback.SensorAspectRatio = InUtuActor.actor_camera.camera_aspect_ratio;
#else
			Comp->FilmbackImportSettings.SensorWidth = InUtuActor.actor_camera.camera_phys_sensor_size.X;
			Comp->FilmbackImportSettings.SensorHeight = InUtuActor.actor_camera.camera_phys_sensor_size.Y;
			Comp->FilmbackImportSettings.SensorAspectRatio = InUtuActor.actor_camera.camera_aspect_ratio;
#endif
		}
		else {
			UTU_LOG_E("            Failed to spawn Cine Camera Actor...");
			UTU_LOG_E("                Actor Name: '" + InUtuActor.actor_display_name + "'");
			UTU_LOG_E("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
			UTU_LOG_E("                Potential Causes:");
			UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
		}
	}
	else {
		UTU_LOG_L("            Adding Camera Actor...");
		UTU_LOG_L("                Actor Name: '" + InUtuActor.actor_display_name + "'");
		UTU_LOG_L("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
		UTU_LOG_L("                Actor Class: 'ACameraActor'");
		UTU_LOG_L("                Actor Tag: '" + InUtuActor.actor_tag + "'");
		FActorSpawnParameters Params = FActorSpawnParameters();
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		RetActor = InAsset->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), Params);
		if (RetActor != nullptr && Cast<ACameraActor>(RetActor)->GetCameraComponent() != nullptr) {
			UCameraComponent* Comp = Cast<ACameraActor>(RetActor)->GetCameraComponent();
			RetActor->SetActorLabel(InUtuActor.actor_display_name);
			//InUtuActor.actor_camera.camera_viewport_rect;
			Comp->OrthoNearClipPlane = InUtuActor.actor_camera.camera_near_clip_plane;
			Comp->OrthoFarClipPlane = InUtuActor.actor_camera.camera_far_clip_plane;
			Comp->AspectRatio = InUtuActor.actor_camera.camera_aspect_ratio;
			Comp->ProjectionMode = InUtuActor.actor_camera.camera_is_perspective ? ECameraProjectionMode::Perspective : ECameraProjectionMode::Orthographic;
			Comp->OrthoWidth = InUtuActor.actor_camera.camera_ortho_size;
			Comp->FieldOfView = InUtuActor.actor_camera.camera_persp_field_of_view;
		}
		else {
			UTU_LOG_E("            Failed to spawn Camera Actor...");
			UTU_LOG_E("                Actor Name: '" + InUtuActor.actor_display_name + "'");
			UTU_LOG_E("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
			UTU_LOG_E("                Potential Causes:");
			UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
		}
	}
	return RetActor;
}

bool FUtuPluginAssetTypeProcessor::BpAddRootComponentForSubComponentsIfNeeded(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode) {
	OutComponentNode = nullptr;
	UTU_LOG_L("        Processing '" + InUniqueName + "'...");
	if (InPrefabComponent.actor_types.Num() != 1) { // If actor_types == 1, we don't need to have an empty root above the other components.
		UTU_LOG_L("            Because there was more than one supported components on this GameObject in Unity, creating another 'Root' to hold them...");
		UTU_LOG_L("                Adding Root component...");
		UTU_LOG_L("                    Component Name: '" + InUniqueName + "'");
		UTU_LOG_L("                    Component ID: " + FString::FromInt(InPrefabComponent.actor_id));
		UTU_LOG_L("                    Component Class: 'USceneComponent'");
		UTU_LOG_L("                    Component Tag: '" + InPrefabComponent.actor_tag + "'");
		// Create Component
		USceneComponent*Component = NewObject<USceneComponent>(InAsset, *InUniqueName);
		Component->SetMobility(InPrefabComponent.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
		Component->ComponentTags.Add(*FString::FromInt(InPrefabComponent.actor_id));
		if (InPrefabComponent.actor_tag != "Untagged") {
			Component->ComponentTags.Add(*InPrefabComponent.actor_tag);
		}
		Component->SetHiddenInGame(!InPrefabComponent.actor_is_visible);
		Component->SetRelativeLocation(UtuConst::ConvertLocation(InPrefabComponent.actor_relative_location));
		Component->SetRelativeRotation(UtuConst::ConvertRotation(InPrefabComponent.actor_relative_rotation));
		Component->SetRelativeScale3D(UtuConst::ConvertScale(InPrefabComponent.actor_relative_scale));
		// Create Component Node
		OutComponentNode = InAsset->SimpleConstructionScript->CreateNode(Component->GetClass(), *InUniqueName);
		UEditorEngine::CopyPropertiesForUnrelatedObjects(Component, OutComponentNode->ComponentTemplate);
		return true;
	}
	return false;
}

void FUtuPluginAssetTypeProcessor::BpAddEmptyComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated) {
	// If we get here, it's only because theres only one empty component to create. Should be almost the same as creating an intermediary root component
	UTU_LOG_L("            Adding Empty component...");
	UTU_LOG_L("                Component Name: '" + InUniqueName + "'");
	UTU_LOG_L("                Component ID: " + FString::FromInt(InPrefabComponent.actor_id));
	UTU_LOG_L("                Component Class: 'USceneComponent'");
	UTU_LOG_L("                Component Tag: '" + InPrefabComponent.actor_tag + "'");
	// Create Component
	USceneComponent* Component = NewObject<USceneComponent>(InAsset, *InUniqueName);
	Component->SetMobility(InPrefabComponent.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
	if (!bInRootCreated) {
		Component->ComponentTags.Add(*FString::FromInt(InPrefabComponent.actor_id));
		if (InPrefabComponent.actor_tag != "Untagged") {
			Component->ComponentTags.Add(*InPrefabComponent.actor_tag);
		}
		Component->SetHiddenInGame(!InPrefabComponent.actor_is_visible);
		Component->SetRelativeLocation(UtuConst::ConvertLocation(InPrefabComponent.actor_relative_location));
		Component->SetRelativeRotation(UtuConst::ConvertRotation(InPrefabComponent.actor_relative_rotation));
		Component->SetRelativeScale3D(UtuConst::ConvertScale(InPrefabComponent.actor_relative_scale));
	}
	// Create Component Node
	OutComponentNode = InAsset->SimpleConstructionScript->CreateNode(Component->GetClass(), *InUniqueName);
	UEditorEngine::CopyPropertiesForUnrelatedObjects(Component, OutComponentNode->ComponentTemplate);
}

void FUtuPluginAssetTypeProcessor::BpAddStaticMeshComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated) {
	UTU_LOG_L("            Adding StaticMesh component...");
	UTU_LOG_L("                Component Name: '" + InUniqueName + "'");
	if (!bInRootCreated) {
		UTU_LOG_L("                Component ID: " + FString::FromInt(InPrefabComponent.actor_id));
	}
	UTU_LOG_L("                Component Class: 'UStaticMeshComponent'");
	UTU_LOG_L("                Component Tag: '" + InPrefabComponent.actor_tag + "'");
	// Create Component
	UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(InAsset, *InUniqueName);
	Component->SetMobility(InPrefabComponent.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
	if (!bInRootCreated) {
		Component->ComponentTags.Add(*FString::FromInt(InPrefabComponent.actor_id));
		if (InPrefabComponent.actor_tag != "Untagged") {
			Component->ComponentTags.Add(*InPrefabComponent.actor_tag);
		}
		Component->SetHiddenInGame(!InPrefabComponent.actor_is_visible);
		Component->SetRelativeLocation(UtuConst::ConvertLocation(ImportSettings.bImportSeparated ? InPrefabComponent.actor_relative_location_if_separated : InPrefabComponent.actor_relative_location));
		Component->SetRelativeRotation(UtuConst::ConvertRotation(InPrefabComponent.actor_relative_rotation));
		Component->SetRelativeScale3D(UtuConst::ConvertScale(InPrefabComponent.actor_relative_scale));
	}
	// Mesh Component Specific
	TArray<FString> MeshNames = FormatRelativeFilenameForUnreal(InPrefabComponent.actor_mesh.actor_mesh_relative_filename);
	TArray<FString> MeshNamesSeparated = FormatRelativeFilenameForUnreal(InPrefabComponent.actor_mesh.actor_mesh_relative_filename_if_separated);
	UStaticMesh* StaticMeshAsset = Cast<UStaticMesh>(UUtuPluginLibrary::TryGetAsset(MeshNames[2]));
	if (StaticMeshAsset == nullptr && ImportSettings.bImportSeparated)
	{
		// Try separated way
		StaticMeshAsset = Cast<UStaticMesh>(UUtuPluginLibrary::TryGetAsset(MeshNamesSeparated[2]));
	}
	Component->SetStaticMesh(StaticMeshAsset);
	if (StaticMeshAsset != nullptr) {
		for (int x = 0; x < InPrefabComponent.actor_mesh.actor_mesh_materials_relative_filenames.Num(); x++) {
			TArray<FString> MatNames = FormatRelativeFilenameForUnreal(InPrefabComponent.actor_mesh.actor_mesh_materials_relative_filenames[x], true);
			UMaterialInterface* MaterialAsset = Cast<UMaterialInterface>(UUtuPluginLibrary::TryGetAsset(MatNames[2]));
			Component->SetMaterial(x, MaterialAsset);
		}
	}
	// Create Component Node
	OutComponentNode = InAsset->SimpleConstructionScript->CreateNode(Component->GetClass(), *InUniqueName);
	UEditorEngine::CopyPropertiesForUnrelatedObjects(Component, OutComponentNode->ComponentTemplate);
}

void FUtuPluginAssetTypeProcessor::BpAddSkeletalMeshComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated) {
	UTU_LOG_L("            Adding SkeletalMesh component...");
	UTU_LOG_L("                Component Name: '" + InUniqueName + "'");
	if (!bInRootCreated) {
		UTU_LOG_L("                Component ID: " + FString::FromInt(InPrefabComponent.actor_id));
	}
	UTU_LOG_L("                Component Class: 'USkeletalMeshComponent'");
	UTU_LOG_L("                Component Tag: '" + InPrefabComponent.actor_tag + "'");
	// Create Component
	USkeletalMeshComponent* Component = NewObject<USkeletalMeshComponent>(InAsset, *InUniqueName);
	Component->SetMobility(InPrefabComponent.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
	if (!bInRootCreated) {
		Component->ComponentTags.Add(*FString::FromInt(InPrefabComponent.actor_id));
		if (InPrefabComponent.actor_tag != "Untagged") {
			Component->ComponentTags.Add(*InPrefabComponent.actor_tag);
		}
		Component->SetHiddenInGame(!InPrefabComponent.actor_is_visible);
		// Do not use the mesh transform since it's driven by the bones (Unity is dumb)
		Component->SetRelativeLocation(UtuConst::ConvertLocation(InPrefabComponent.actor_relative_location));
		Component->SetRelativeRotation(UtuConst::ConvertRotation(InPrefabComponent.actor_relative_rotation));
		Component->SetRelativeScale3D(UtuConst::ConvertScale(InPrefabComponent.actor_relative_scale));
	}
	// Mesh Component Specific
	TArray<FString> MeshNames = FormatRelativeFilenameForUnreal(InPrefabComponent.actor_mesh.actor_mesh_relative_filename);
	USkeletalMesh* SkeletalMeshAsset = Cast<USkeletalMesh>(UUtuPluginLibrary::TryGetAsset(MeshNames[2]));
	Component->SetSkeletalMesh(SkeletalMeshAsset);
	if (SkeletalMeshAsset != nullptr) {
		for (int x = 0; x < InPrefabComponent.actor_mesh.actor_mesh_materials_relative_filenames.Num(); x++) {
			TArray<FString> MatNames = FormatRelativeFilenameForUnreal(InPrefabComponent.actor_mesh.actor_mesh_materials_relative_filenames[x], true);
			UMaterialInterface* MaterialAsset = Cast<UMaterialInterface>(UUtuPluginLibrary::TryGetAsset(MatNames[2]));
			Component->SetMaterial(x, MaterialAsset);
		}
	}
	// Create Component Node
	OutComponentNode = InAsset->SimpleConstructionScript->CreateNode(Component->GetClass(), *InUniqueName);
	UEditorEngine::CopyPropertiesForUnrelatedObjects(Component, OutComponentNode->ComponentTemplate);
}

void FUtuPluginAssetTypeProcessor::BpAddPointLightComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated) {
	UTU_LOG_L("            Adding PointLight component...");
	UTU_LOG_L("                Component Name: '" + InUniqueName + "'");
	if (!bInRootCreated) {
		UTU_LOG_L("                Component ID: " + FString::FromInt(InPrefabComponent.actor_id));
	}
	UTU_LOG_L("                Component Class: 'UPointLightComponent'");
	UTU_LOG_L("                Component Tag: '" + InPrefabComponent.actor_tag + "'");
	// Create Component
	UPointLightComponent* Component = NewObject<UPointLightComponent>(InAsset, *InUniqueName);
	Component->SetMobility(InPrefabComponent.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
	if (!bInRootCreated) {
		Component->ComponentTags.Add(*FString::FromInt(InPrefabComponent.actor_id));
		if (InPrefabComponent.actor_tag != "Untagged") {
			Component->ComponentTags.Add(*InPrefabComponent.actor_tag);
		}
		Component->SetHiddenInGame(!InPrefabComponent.actor_is_visible);
		Component->SetRelativeLocation(UtuConst::ConvertLocation(InPrefabComponent.actor_relative_location));
		Component->SetRelativeRotation(UtuConst::ConvertRotation(InPrefabComponent.actor_relative_rotation));
		Component->SetRelativeScale3D(UtuConst::ConvertScale(InPrefabComponent.actor_relative_scale));
	}
	// Point Light Component Specific
	Component->Intensity = InPrefabComponent.actor_light.light_intensity * ImportSettings.LightIntensityMultiplier;
	Component->bUseInverseSquaredFalloff = false;
	Component->SetLightFalloffExponent(ImportSettings.LightFalloffExponent);
	Component->SetLightColor(HexToColor(InPrefabComponent.actor_light.light_color));
	Component->AttenuationRadius = InPrefabComponent.actor_light.light_range * ImportSettings.LightRangeMultiplier;
	Component->SetAttenuationRadius(InPrefabComponent.actor_light.light_range * ImportSettings.LightRangeMultiplier);
	Component->CastShadows = InPrefabComponent.actor_light.light_is_casting_shadows;
	// Create Component Node
	OutComponentNode = InAsset->SimpleConstructionScript->CreateNode(Component->GetClass(), *InUniqueName);
	UEditorEngine::CopyPropertiesForUnrelatedObjects(Component, OutComponentNode->ComponentTemplate);
}

void FUtuPluginAssetTypeProcessor::BpAddDirectionalLightComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated) {
	UTU_LOG_L("            Adding DirectionalLight component...");
	UTU_LOG_L("                Component Name: '" + InUniqueName + "'");
	if (!bInRootCreated) {
		UTU_LOG_L("                Component ID: " + FString::FromInt(InPrefabComponent.actor_id));
	}
	UTU_LOG_L("                Component Class: 'UDirectionalLightComponent'");
	UTU_LOG_L("                Component Tag: '" + InPrefabComponent.actor_tag + "'");
	// Create Component
	UDirectionalLightComponent* Component = NewObject<UDirectionalLightComponent>(InAsset, *InUniqueName);
	Component->SetMobility(InPrefabComponent.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
	if (!bInRootCreated) {
		Component->ComponentTags.Add(*FString::FromInt(InPrefabComponent.actor_id));
		if (InPrefabComponent.actor_tag != "Untagged") {
			Component->ComponentTags.Add(*InPrefabComponent.actor_tag);
		}
		Component->SetHiddenInGame(!InPrefabComponent.actor_is_visible);
		Component->SetRelativeLocation(UtuConst::ConvertLocation(InPrefabComponent.actor_relative_location));
		Component->SetRelativeRotation(UtuConst::ConvertRotation(InPrefabComponent.actor_relative_rotation));
		Component->SetRelativeScale3D(UtuConst::ConvertScale(InPrefabComponent.actor_relative_scale));
	}
	// Point Light Component Specific
	Component->Intensity = InPrefabComponent.actor_light.light_intensity * ImportSettings.LightIntensityMultiplier;
	Component->SetLightColor(HexToColor(InPrefabComponent.actor_light.light_color));
	Component->CastShadows = InPrefabComponent.actor_light.light_is_casting_shadows;
	// Create Component Node
	OutComponentNode = InAsset->SimpleConstructionScript->CreateNode(Component->GetClass(), *InUniqueName);
	UEditorEngine::CopyPropertiesForUnrelatedObjects(Component, OutComponentNode->ComponentTemplate);
}

void FUtuPluginAssetTypeProcessor::BpAddSpotLightComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated) {
	UTU_LOG_L("            Adding SpotLight component...");
	UTU_LOG_L("                Component Name: '" + InUniqueName + "'");
	if (!bInRootCreated) {
		UTU_LOG_L("                Component ID: " + FString::FromInt(InPrefabComponent.actor_id));
	}
	UTU_LOG_L("                Component Class: 'USpotLightComponent'");
	UTU_LOG_L("                Component Tag: '" + InPrefabComponent.actor_tag + "'");
	// Create Component
	USpotLightComponent* Component = NewObject<USpotLightComponent>(InAsset, *InUniqueName);
	Component->SetMobility(InPrefabComponent.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
	if (!bInRootCreated) {
		Component->ComponentTags.Add(*FString::FromInt(InPrefabComponent.actor_id));
		if (InPrefabComponent.actor_tag != "Untagged") {
			Component->ComponentTags.Add(*InPrefabComponent.actor_tag);
		}
		Component->SetHiddenInGame(!InPrefabComponent.actor_is_visible);
		Component->SetRelativeLocation(UtuConst::ConvertLocation(InPrefabComponent.actor_relative_location));
		Component->SetRelativeRotation(UtuConst::ConvertRotation(InPrefabComponent.actor_relative_rotation));
		Component->SetRelativeScale3D(UtuConst::ConvertScale(InPrefabComponent.actor_relative_scale));
	}
	// Point Light Component Specific
	Component->Intensity = InPrefabComponent.actor_light.light_intensity * ImportSettings.LightIntensityMultiplier;
	Component->bUseInverseSquaredFalloff = false;
	Component->SetLightFalloffExponent(ImportSettings.LightFalloffExponent);
	Component->SetLightColor(HexToColor(InPrefabComponent.actor_light.light_color));
	Component->AttenuationRadius = InPrefabComponent.actor_light.light_range * ImportSettings.LightRangeMultiplier;
	Component->SetAttenuationRadius(InPrefabComponent.actor_light.light_range * ImportSettings.LightRangeMultiplier);
	Component->CastShadows = InPrefabComponent.actor_light.light_is_casting_shadows;
	Component->InnerConeAngle = ImportSettings.LightSpotInnerConeAngle;
	Component->OuterConeAngle = InPrefabComponent.actor_light.light_spot_angle * ImportSettings.LightSpotAngleMultiplier;
	// Create Component Node
	OutComponentNode = InAsset->SimpleConstructionScript->CreateNode(Component->GetClass(), *InUniqueName);
	UEditorEngine::CopyPropertiesForUnrelatedObjects(Component, OutComponentNode->ComponentTemplate);
}

void FUtuPluginAssetTypeProcessor::BpAddCameraComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated) {
	if (InPrefabComponent.actor_camera.camera_is_physical) {
		UTU_LOG_L("            Adding Cine Camera Component...");
		UTU_LOG_L("                Component Name: '" + InUniqueName + "'");
		if (!bInRootCreated) {
			UTU_LOG_L("                Component ID: " + FString::FromInt(InPrefabComponent.actor_id));
		}
		UTU_LOG_L("                Component Class: 'UCineCameraComponent'");
		UTU_LOG_L("                Component Tag: '" + InPrefabComponent.actor_tag + "'");
		// Create Component
		UCineCameraComponent* Component = NewObject<UCineCameraComponent>(InAsset, *InUniqueName);
		Component->SetMobility(InPrefabComponent.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
		if (!bInRootCreated) {
			Component->ComponentTags.Add(*FString::FromInt(InPrefabComponent.actor_id));
			if (InPrefabComponent.actor_tag != "Untagged") {
				Component->ComponentTags.Add(*InPrefabComponent.actor_tag);
			}
			Component->SetHiddenInGame(!InPrefabComponent.actor_is_visible);
			Component->SetRelativeLocation(UtuConst::ConvertLocation(InPrefabComponent.actor_relative_location));
			Component->SetRelativeRotation(UtuConst::ConvertRotation(InPrefabComponent.actor_relative_rotation));
			Component->SetRelativeScale3D(UtuConst::ConvertScale(InPrefabComponent.actor_relative_scale));
		}
		// Cine Camera Component Specific
		//InPrefabComponent.actor_camera.camera_viewport_rect;
		Component->OrthoNearClipPlane = InPrefabComponent.actor_camera.camera_near_clip_plane;
		Component->OrthoFarClipPlane = InPrefabComponent.actor_camera.camera_far_clip_plane;
		Component->AspectRatio = InPrefabComponent.actor_camera.camera_aspect_ratio;
		Component->ProjectionMode = InPrefabComponent.actor_camera.camera_is_perspective ? ECameraProjectionMode::Perspective : ECameraProjectionMode::Orthographic;
		Component->OrthoWidth = InPrefabComponent.actor_camera.camera_ortho_size;
		Component->FieldOfView = InPrefabComponent.actor_camera.camera_persp_field_of_view;
		Component->CurrentFocalLength = InPrefabComponent.actor_camera.camera_phys_focal_length;
#if ENGINE_MINOR_VERSION >= 24 || ENGINE_MAJOR_VERSION >= 5
		Component->Filmback.SensorWidth = InPrefabComponent.actor_camera.camera_phys_sensor_size.X;
		Component->Filmback.SensorHeight = InPrefabComponent.actor_camera.camera_phys_sensor_size.Y;
		Component->Filmback.SensorAspectRatio = InPrefabComponent.actor_camera.camera_aspect_ratio;
#else
		Component->FilmbackImportSettings.SensorWidth = InPrefabComponent.actor_camera.camera_phys_sensor_size.X;
		Component->FilmbackImportSettings.SensorHeight = InPrefabComponent.actor_camera.camera_phys_sensor_size.Y;
		Component->FilmbackImportSettings.SensorAspectRatio = InPrefabComponent.actor_camera.camera_aspect_ratio;
#endif
		// Create Component Node
		OutComponentNode = InAsset->SimpleConstructionScript->CreateNode(Component->GetClass(), *InUniqueName);
		UEditorEngine::CopyPropertiesForUnrelatedObjects(Component, OutComponentNode->ComponentTemplate);
	}
	else {
		UTU_LOG_L("            Adding Camera Actor...");
		UTU_LOG_L("                Component Name: '" + InUniqueName + "'");
		if (!bInRootCreated) {
			UTU_LOG_L("                Component ID: " + FString::FromInt(InPrefabComponent.actor_id));
		}
		UTU_LOG_L("                Component Class: 'UCameraComponent'");
		UTU_LOG_L("                Component Tag: '" + InPrefabComponent.actor_tag + "'");
		// Create Component
		UCameraComponent* Component = NewObject<UCameraComponent>(InAsset, *InUniqueName);
		Component->SetMobility(InPrefabComponent.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
		if (!bInRootCreated) {
			Component->ComponentTags.Add(*FString::FromInt(InPrefabComponent.actor_id));
			if (InPrefabComponent.actor_tag != "Untagged") {
				Component->ComponentTags.Add(*InPrefabComponent.actor_tag);
			}
			Component->SetHiddenInGame(!InPrefabComponent.actor_is_visible);
			Component->SetRelativeLocation(UtuConst::ConvertLocation(InPrefabComponent.actor_relative_location));
			Component->SetRelativeRotation(UtuConst::ConvertRotation(InPrefabComponent.actor_relative_rotation));
			Component->SetRelativeScale3D(UtuConst::ConvertScale(InPrefabComponent.actor_relative_scale));
		}
		// Camera Component Specific
		//InPrefabComponent.actor_camera.camera_viewport_rect;
		Component->OrthoNearClipPlane = InPrefabComponent.actor_camera.camera_near_clip_plane;
		Component->OrthoFarClipPlane = InPrefabComponent.actor_camera.camera_far_clip_plane;
		Component->AspectRatio = InPrefabComponent.actor_camera.camera_aspect_ratio;
		Component->ProjectionMode = InPrefabComponent.actor_camera.camera_is_perspective ? ECameraProjectionMode::Perspective : ECameraProjectionMode::Orthographic;
		Component->OrthoWidth = InPrefabComponent.actor_camera.camera_ortho_size;
		Component->FieldOfView = InPrefabComponent.actor_camera.camera_persp_field_of_view;
		// Create Component Node
		OutComponentNode = InAsset->SimpleConstructionScript->CreateNode(Component->GetClass(), *InUniqueName);
		UEditorEngine::CopyPropertiesForUnrelatedObjects(Component, OutComponentNode->ComponentTemplate);
	}
}

void FUtuPluginAssetTypeProcessor::BpAddChildActorComponent(UBlueprint * InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated) {
	UTU_LOG_L("            Adding Prefab component...");
	UTU_LOG_L("                Component Name: '" + InUniqueName + "'");
	if (!bInRootCreated) {
		UTU_LOG_L("                Component ID: " + FString::FromInt(InPrefabComponent.actor_id));
	}
	UTU_LOG_L("                Component Class: 'UChildActorComponent'");
	UTU_LOG_L("                Component Tag: '" + InPrefabComponent.actor_tag + "'");
	// Create Component
	UChildActorComponent* Component = NewObject<UChildActorComponent>(InAsset, *InUniqueName);
	Component->SetMobility(InPrefabComponent.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
	if (!bInRootCreated) {
		Component->ComponentTags.Add(*FString::FromInt(InPrefabComponent.actor_id));
		if (InPrefabComponent.actor_tag != "Untagged") {
			Component->ComponentTags.Add(*InPrefabComponent.actor_tag);
		}
		Component->SetHiddenInGame(!InPrefabComponent.actor_is_visible);
		Component->SetRelativeLocation(UtuConst::ConvertLocation(InPrefabComponent.actor_relative_location));
		Component->SetRelativeRotation(UtuConst::ConvertRotation(InPrefabComponent.actor_relative_rotation));
		Component->SetRelativeScale3D(UtuConst::ConvertScale(InPrefabComponent.actor_relative_scale));
	}
	// Child Actor Specific
	TArray<FString> BpNames = FormatRelativeFilenameForUnreal(InPrefabComponent.actor_prefab.actor_prefab_relative_filename);
	UBlueprint* BpChild = Cast<UBlueprint>(UUtuPluginLibrary::TryGetAsset(BpNames[2]));
	Component->SetChildActorClass(TSubclassOf<AActor>(BpChild->GeneratedClass));
	// Create Component Node
	OutComponentNode = InAsset->SimpleConstructionScript->CreateNode(Component->GetClass(), *InUniqueName);
	UEditorEngine::CopyPropertiesForUnrelatedObjects(Component, OutComponentNode->ComponentTemplate);
}

FString FUtuPluginAssetTypeProcessor::BpMakeUniqueName(FString InDesiredName, TArray<FString>& InOutUsedNames) {
	if (InOutUsedNames.Contains(InDesiredName)) {
		for (int x = 1; x < 1000; x++) {
			FString PotentialName = InDesiredName + "_" + FString::FromInt(x);
			if (!InOutUsedNames.Contains(PotentialName)) {
				InOutUsedNames.Add(PotentialName);
				return PotentialName;
			}
		}
	}
	InOutUsedNames.Add(InDesiredName);
	return InDesiredName;
}


UAssetImportTask* FUtuPluginAssetTypeProcessor::BuildTask(FString InSource, TArray<FString> InAssetNames, UObject* InOptions) {
	UAssetImportTask* RetTask = NewObject<UAssetImportTask>();
	RetTask->Filename = InSource;
	RetTask->DestinationPath = InAssetNames[0];
	RetTask->DestinationName = InAssetNames[1];
	RetTask->bSave = false;
	RetTask->bAutomated = true;
	RetTask->bReplaceExisting = true;
	RetTask->Options = InOptions;
	return RetTask;
}

TArray<FString> FUtuPluginAssetTypeProcessor::StartProcessAsset(FUtuPluginAsset InUtuAsset, bool bInIsMaterial) {
	TArray<FString> RetAssetNames = FormatRelativeFilenameForUnreal(InUtuAsset.asset_relative_filename, bInIsMaterial);
	UTU_LOG_EMPTY_LINE();
	UTU_LOG_L("Asset Name: " + RetAssetNames[1]);
	UTU_LOG_L("    Unity  Asset Relative Path: " + InUtuAsset.asset_relative_filename);
	UTU_LOG_L("    Unreal Asset Relative Path: " + RetAssetNames[2]);
	UTU_LOG_L("Begin Creation ...");
	UTU_LOG_L("    Time: " + FDateTime::UtcNow().ToString());
	return RetAssetNames;
}

bool FUtuPluginAssetTypeProcessor::DeleteInvalidAssetIfNeeded(TArray<FString> InAssetNames, UClass* InClass) {
	UObject* Asset = UUtuPluginLibrary::TryGetAsset(InAssetNames[2]);
	if (Asset != nullptr) {
		if (Asset->GetClass() != InClass) {
			if (bDeleteInvalidAssets) {
				UTU_LOG_L("    Existing Invalid Asset detected, deleting ...");
				if (UUtuPluginLibrary::DeleteAsset(Asset)) {
					UTU_LOG_L("        Invalid Asset deleted.");
				}
				else {
					UTU_LOG_E("    Asset Name: " + InAssetNames[1]);
					UTU_LOG_E("        Unreal Asset Relative Path: " + InAssetNames[2]);
					UTU_LOG_E("            You are trying to import a '" + InClass->GetDisplayNameText().ToString() + "' over an already existing asset that is of type '" + Asset->GetClass()->GetDisplayNameText().ToString() + "'.");
					UTU_LOG_E("            Tried to delete the Invalid Asset, but failed to delete it.");
					UTU_LOG_E("            Potential Causes:");
					UTU_LOG_E("                - The asset might be referenced by other content. Please delete it manually.");
					UTU_LOG_E("            Asset skipped.");
					return false;
				}
			}
			else {
				UTU_LOG_W("    Asset Name: " + InAssetNames[1]);
				UTU_LOG_W("        Unreal Asset Relative Path: " + InAssetNames[2]);
				UTU_LOG_W("            You are trying to import a '" + InClass->GetDisplayNameText().ToString() + "' over an already existing asset that is of type '" + Asset->GetClass()->GetDisplayNameText().ToString() + "'.");
				UTU_LOG_W("            If you want to process this asset, please delete the current asset or enable the 'Delete Invalid Assets' functionnality.");
				UTU_LOG_W("            Asset skipped.");
				return false;
			}
		}
	}
	return true;
}

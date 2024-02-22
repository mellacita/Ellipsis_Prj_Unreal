// Copyright Alex Quevillon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UTUPLUGIN_API UtuConst {
public:
	static const int INVALID_INT;
	static const FString DEFAULT_RESOURCES;

public:
	static FVector ConvertLocation(FVector Vector, bool bIsForMeshImport = false, FQuat InMeshImportRotation = {});
	static FQuat ConvertRotation(FQuat Quat, bool bIsForMeshImport = false);
	static FVector ConvertScale(FVector Vector);
};
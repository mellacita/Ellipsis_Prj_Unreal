// Copyright Alex Quevillon. All Rights Reserved.

#include "UtuPlugin/Scripts/Public/UtuPluginConstants.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"

const int UtuConst::INVALID_INT = -999;
const FString UtuConst::DEFAULT_RESOURCES = "unity default resources";

FVector UtuConst::ConvertLocation(FVector Vector, bool bIsForMeshImport, FQuat InMeshImportRotation) {
	if (bIsForMeshImport) {
		FVector X, Y, Z;
		UKismetMathLibrary::BreakRotIntoAxes(InMeshImportRotation.Rotator(), X, Y, Z);
		FRotator Rotation = UKismetMathLibrary::MakeRotationFromAxes(Z, X, Y);
		return -UKismetMathLibrary::LessLess_VectorRotator(FVector(Vector.X, Vector.Y, Vector.Z) * 100.0f, Rotation);
	}
	return FVector(Vector.Z, Vector.X, Vector.Y) * 100.0f;
}

FQuat UtuConst::ConvertRotation(FQuat Quat, bool bIsForMeshImport) {
	if (bIsForMeshImport) {
		return (FQuat(Quat.Z, Quat.X, Quat.Y, -Quat.W));
	}
	return (FQuat(Quat.Z, Quat.X, Quat.Y, Quat.W));
}

FVector UtuConst::ConvertScale(FVector Vector) {
	return FVector(Vector.Z, Vector.X, Vector.Y);
}

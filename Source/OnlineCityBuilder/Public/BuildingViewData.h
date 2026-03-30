#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BuildingViewData.generated.h"

UCLASS()
class ONLINECITYBUILDER_API UBuildingViewData : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere) UStaticMesh* windowMesh;
	UPROPERTY(EditAnywhere) UStaticMesh* cornerMesh;
	UPROPERTY(EditAnywhere) UStaticMesh* ceilingBorderMesh;
	UPROPERTY(EditAnywhere) UStaticMesh* ceilingCornerMesh;
	UPROPERTY(EditAnywhere) UStaticMesh* ceilingFloorMesh;
	UPROPERTY(EditAnywhere) UMaterial*	 mat;
};

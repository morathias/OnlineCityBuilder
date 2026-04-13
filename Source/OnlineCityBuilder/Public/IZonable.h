// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IZonable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UZonable : public UInterface
{
	GENERATED_BODY()
};

class Zone;
/**
 * 
 */
class ONLINECITYBUILDER_API IZonable
{
	GENERATED_BODY()

public:
	TArray<Zone*> attachedZones;

	virtual TArray<IZonable*> GetNeighbourZonables() = 0;

	virtual TArray<FVector> GetRightEdge() = 0;
	virtual TArray<FVector> GetRightEdgeNormals() = 0;

	virtual TArray<FVector> GetLeftEdge() = 0;
	virtual TArray<FVector> GetLeftEdgeNormals() = 0;
};

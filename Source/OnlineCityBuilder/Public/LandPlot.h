// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class ABuilding;

class ONLINECITYBUILDER_API LandPlot
{
public:
	LandPlot(TArray<FVector*> startingBorder, FVector pathDir);
	~LandPlot();

	TArray<FVector*> GetLeftBorder();
	TArray<FVector>& GetVertices();
	FVector& GetCenter() { return center; }

private:
	void ConstructBuilding();

	TArray<FVector> vertices;
	FVector center;

	ABuilding* building;
};

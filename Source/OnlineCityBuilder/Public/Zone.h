// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IZonable.h"

class LandPlot;

class ONLINECITYBUILDER_API Zone
{
public:
	Zone(TArray<FVector> vertices, TArray<FVector> normals, IZonable* owner);
	~Zone();

	void RecalculateVertices(TArray<FVector> newVertices, TArray<FVector> normals, bool updateLandPLotes = false);
	TArray<FVector> GetVertices();
	TArray<int> GetIndices();
	const TArray<LandPlot*>& GetLandPlots();

	float width = 2500;


private:
	void GenerateLandPlots();
	void UpdateLandPlots();

	float AngleBetweenVectors(FVector Vec1, FVector Vec2);

	TArray<FVector> vertices;
	TArray<int> indices;

	TArray<LandPlot*> landPlots;

	IZonable* owner;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class ONLINECITYBUILDER_API Zone
{
public:
	Zone(TArray<FVector> vertices, TArray<FVector> normals, int index);
	~Zone();

	void RecalculateVertices(TArray<FVector> newVertices, TArray<FVector> normals, int index);
	TArray<FVector> GetVertices();
	TArray<int> GetIndices();

	float width = 2500;


private:
	TArray<FVector> vertices;
	TArray<int> indices;
};

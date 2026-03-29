#include "Zone.h"
#include "LandPlot.h"

Zone::Zone(TArray<FVector> vertices, TArray<FVector> normals, int index)
{
	RecalculateVertices(vertices, normals, index);
	GenerateLandPlots();
}

Zone::~Zone()
{
	for (LandPlot* plot : landPlots) 
	{
		delete plot;
	}
}

void Zone::RecalculateVertices(TArray<FVector> newVertices, TArray<FVector> normals, int index)
{
	vertices.Empty();
	indices.Empty();

	for (int i = 1; i < newVertices.Num(); i++)
	{
		FVector extrudedVertex = newVertices[i - 1] + normals[i - 1] * width;
		this->vertices.AddUnique(newVertices[i - 1]);
		this->vertices.AddUnique(extrudedVertex);

		FVector nextExtrudedVertex = newVertices[i] + normals[i] * width;
		this->vertices.AddUnique(newVertices[i]);
		this->vertices.AddUnique(nextExtrudedVertex);
	}

	for (int i = 3; i < vertices.Num(); i += 2)
	{
		indices.Add(i - 3);
		indices.Add(i - 2);
		indices.Add(i - 1);

		indices.Add(i - 1);
		indices.Add(i - 2);
		indices.Add(i);
	}
}

TArray<FVector> Zone::GetVertices() 
{
	return vertices;
}

TArray<int> Zone::GetIndices()
{
	return indices;
}

void Zone::GenerateLandPlots() 
{
	int32 plots = FMath::RandRange(4, 6);
	//int32 plots = 2;

	TArray<FVector*> border;
	border.Add(&vertices[0]);
	border.Add(&vertices[1]);

	FVector dir = vertices[2] - vertices[0];
	float zoneLength = dir.Length();
	dir.Normalize();
	float plotWidth = (float)(zoneLength / plots);
	
	FVector pathDir = dir * plotWidth;

	LandPlot* landPlot = new LandPlot(border, pathDir);

	landPlots.Add(landPlot);

	for (int32 i = 1; i < plots; i++)
	{
		TArray<FVector*> nextBorder;
		nextBorder.Append(landPlots[landPlots.Num() - 1]->GetLeftBorder());

		//TODO: this should consider the zone's shape but is not implemented yet
		FVector nextPathDir = pathDir;
		
		LandPlot* nextLandPlot = new LandPlot(nextBorder, nextPathDir);

		landPlots.Add(nextLandPlot);
	}
}

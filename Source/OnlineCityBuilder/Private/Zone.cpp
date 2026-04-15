#include "Zone.h"
#include "LandPlot.h"

Zone::Zone(TArray<FVector> vertices, TArray<FVector> normals, IZonable* owner)
{
	this->owner = owner;

	this->owner->attachedZones.Add(this);

	RecalculateVertices(vertices, normals);
	GenerateLandPlots();
}

Zone::~Zone()
{
	for (LandPlot* plot : landPlots) 
	{
		delete plot;
	}
}

void Zone::RecalculateVertices(TArray<FVector> newVertices, TArray<FVector> normals, bool updateLandPlots)
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

	if (updateLandPlots) 
	{
		UpdateLandPlots();
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

float Zone::AngleBetweenVectors(FVector Vec1, FVector Vec2)
{
	// Normalize vectors to get direction only (magnitude becomes 1)
	Vec1.Normalize();
	Vec2.Normalize();

	// Calculate the dot product of the two normalized vectors
	float DotProduct = FVector::DotProduct(Vec1, Vec2);

	// Calculate the angle in radians using Acos.
	// The DotProduct is the cosine of the angle.
	float AngleRadians = FMath::Acos(DotProduct);

	// Optional: Convert radians to degrees
	float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	return AngleDegrees; // Or return AngleRadians if you need radians
}

void Zone::GenerateLandPlots() 
{
	//int32 plots = FMath::RandRange(2, 6);
	int32 plots = 1;

	TArray<FVector> leftBorder;
	leftBorder.Add(vertices[0]);
	leftBorder.Add(vertices[1]);

	FVector dir = vertices[2] - vertices[0];
	float zoneLength = dir.Length();
	dir.Normalize();

	float plotWidth = (float)(zoneLength / plots);
	
	FVector pathDir = dir * plotWidth;

	TArray<FVector> rightBorder;
	rightBorder.Add(vertices[0] + pathDir);
	rightBorder.Add(rightBorder[0] + (vertices[3] - vertices[2]));


	LandPlot* landPlot = new LandPlot(leftBorder, rightBorder);

	landPlots.Add(landPlot);

	for (int32 i = 1; i < plots - 1; i++)
	{
		TArray<FVector> nextBorder;
		nextBorder.Append(landPlots[landPlots.Num() - 1]->GetLeftBorder());

		TArray<FVector> nextRightBorder;
		nextRightBorder.Add(nextBorder[0] + pathDir);
		nextRightBorder.Add(nextBorder[1] + pathDir);

		
		LandPlot* nextLandPlot = new LandPlot(nextBorder, nextRightBorder);

		landPlots.Add(nextLandPlot);
	}

	if (plots <= 1) return;

	TArray<FVector> lastLeftBorder;
	lastLeftBorder.Append(landPlots[landPlots.Num() - 1]->GetLeftBorder());

	TArray<FVector> lastBorder;
	lastBorder.Add(vertices[2]);
	lastBorder.Add(vertices[3]);

	LandPlot* lastLandPlot = new LandPlot(lastLeftBorder, lastBorder);

	landPlots.Add(lastLandPlot);
}

const TArray<LandPlot*>& Zone::GetLandPlots() 
{
	return landPlots;
}

void Zone::UpdateLandPlots() 
{
	TArray<FVector> leftBorder;
	leftBorder.Add(vertices[0]);
	leftBorder.Add(vertices[1]);

	FVector dir = vertices[2] - vertices[0];
	float zoneLength = dir.Length();
	dir.Normalize();

	float plotWidth = (float)(zoneLength / landPlots.Num());

	FVector pathDir = dir * plotWidth;

	TArray<FVector> rightBorder;
	rightBorder.Add(vertices[0] + pathDir);
	rightBorder.Add(rightBorder[0] + (vertices[3] - vertices[2]));

	landPlots[0]->UpdateVertices(leftBorder, rightBorder);

	for (int32 i = 1; i < landPlots.Num() - 1; i++)
	{
		TArray<FVector> nextBorder;
		nextBorder.Append(landPlots[landPlots.Num() - 1]->GetLeftBorder());

		TArray<FVector> nextRightBorder;
		nextRightBorder.Add(nextBorder[0] + pathDir);
		nextRightBorder.Add(nextBorder[1] + pathDir);

		landPlots[i]->UpdateVertices(nextBorder, nextRightBorder);
	}

	if (landPlots.Num() <= 1) return;

	TArray<FVector> lastLeftBorder;
	lastLeftBorder.Append(landPlots[landPlots.Num() - 1]->GetLeftBorder());

	TArray<FVector> lastBorder;
	lastBorder.Add(vertices[2]);
	lastBorder.Add(vertices[3]);

	landPlots[landPlots.Num() - 1]->UpdateVertices(lastLeftBorder, lastBorder);
}
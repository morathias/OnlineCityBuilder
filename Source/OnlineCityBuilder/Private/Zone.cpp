#include "Zone.h"

Zone::Zone(TArray<FVector> vertices, TArray<FVector> normals, int index)
{
	RecalculateVertices(vertices, normals, index);
}

Zone::~Zone(){}

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

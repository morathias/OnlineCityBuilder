#include "LandPlot.h"
#include "ABuilding.h"
#include "MyCityBuilderGameMode.h"
#include <DrawDebugHelpers.h>


#include <Engine/World.h>

LandPlot::LandPlot(TArray<FVector> leftBorder, TArray<FVector> rightBorder)
{
	vertices.Add(leftBorder[0]);
	vertices.Add(rightBorder[0]);
	vertices.Add(leftBorder[1]);
	vertices.Add(rightBorder[1]);

	DrawDebugSphere(GWorld, vertices[0], 300, 4, FColor::Red, true);
	DrawDebugSphere(GWorld, vertices[1], 300, 4, FColor::Red, true);
	DrawDebugSphere(GWorld, vertices[2], 300, 4, FColor::Red, true);
	DrawDebugSphere(GWorld, vertices[3], 300, 4, FColor::Red, true);
	//DrawDebugSphere(GWorld, vertices[0], 300, 4, FColor::Black, false);

	ConstructBuilding();
}

LandPlot::~LandPlot()
{
	delete building;
}

TArray<FVector> LandPlot::GetLeftBorder() 
{
	TArray<FVector> leftBorder;

	leftBorder.Add(vertices[1]);
	leftBorder.Add(vertices[3]);

	return leftBorder;
}

void LandPlot::ConstructBuilding() 
{
	AMyCityBuilderGameMode* gameMode = Cast<AMyCityBuilderGameMode>(GWorld->GetAuthGameMode());

	if (gameMode != nullptr) 
	{
		//TODO: find a way to remove this debug, removing it breacks everything for some reason
		DrawDebugSphere(GWorld, vertices[0], 300, 4, FColor::Black, false);
		//DrawDebugSphere(GWorld, vertices[1], 300, 4, FColor::Black, true);
		//DrawDebugSphere(GWorld, vertices[2], 300, 4, FColor::Black, true);
		//DrawDebugSphere(GWorld, vertices[3], 300, 4, FColor::Black, true);

		building = GWorld->SpawnActor<ABuilding>(gameMode->buildingTypes[0]);
		building->StartBuilding(vertices);
		center = building->GetActorLocation();
	}
}

TArray<FVector>& LandPlot::GetVertices() { return vertices; }

void LandPlot::UpdateVertices(TArray<FVector> leftBorder, TArray<FVector> rightBorder)
{
	vertices.Empty();

	vertices.Add(leftBorder[0]);
	vertices.Add(rightBorder[0]);
	vertices.Add(leftBorder[1]);
	vertices.Add(rightBorder[1]);
}

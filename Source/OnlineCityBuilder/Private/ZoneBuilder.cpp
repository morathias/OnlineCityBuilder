#include "ZoneBuilder.h"

#include <RealtimeMeshSimple.h>
#include <RealtimeMeshComponent.h>
#include <Debug/DebugDrawComponent.h>
#include "LandPlot.h"

// Sets default values
AZoneBuilder::AZoneBuilder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	mesh = CreateDefaultSubobject<URealtimeMeshComponent>("ZonesMesh");
	SetRootComponent(mesh);

}

// Called when the game starts or when spawned
void AZoneBuilder::BeginPlay()
{
	Super::BeginPlay();
	generatedMesh = mesh->InitializeRealtimeMesh<URealtimeMeshSimple>();
}

// Called every frame
void AZoneBuilder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AZoneBuilder::SetMaterial(UMaterialInterface* mat) 
{
	zoneMaterial = mat;
	mesh->SetMaterial(0, zoneMaterial);
}

void AZoneBuilder::GenerateZone(TArray<FVector> vertices, TArray<FVector> normals, IZonable* owner) 
{
	Zone* newZone = new Zone(vertices, normals, owner);
	zones.Add(newZone);

	CalculateMesh();
}

void AZoneBuilder::UpdateZone(TArray<FVector> vertices, TArray<FVector> normals, Zone* zoneToUpdate)
{
	zoneToUpdate->RecalculateVertices(vertices, normals, true);

	CalculateMesh();
}

void AZoneBuilder::CalculateMesh() 
{
	meshVertices.Empty();
	meshIndices.Empty();
	TArray<FColor> meshColors;

	TArray<int> plotIndices = { 0,4,1,2,4,0,3,4,2,1,4,3 };

	for (Zone* zone : zones) 
	{
		for(LandPlot* plot : zone->GetLandPlots())
		{
			for (int32 i = 0; i < plotIndices.Num(); i++)
				meshIndices.Add(plotIndices[i] + meshVertices.Num());

			meshVertices.Append(plot->GetVertices());
			meshVertices.Add(plot->GetCenter());
			meshColors.Append({ FColor::White, FColor::White , FColor::White , FColor::White , FColor::Black });
		}
	}

	FRealtimeMeshSimpleMeshData LODMeshData;
	LODMeshData.Positions = meshVertices;
	LODMeshData.Triangles = meshIndices;
	LODMeshData.Colors = meshColors;

	FRealtimeMeshSectionGroupKey groupKey = FRealtimeMeshSectionGroupKey::Create(FRealtimeMeshLODKey(0), "Zones");

	if (generatedMesh->GetSectionGroup(groupKey) != nullptr)
	{
		generatedMesh->UpdateSectionGroup(groupKey, LODMeshData);
		return;
	}

	generatedMesh->CreateSectionGroup(groupKey, LODMeshData);
}



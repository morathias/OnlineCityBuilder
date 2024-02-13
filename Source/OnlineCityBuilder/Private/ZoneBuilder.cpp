#include "ZoneBuilder.h"

#include <ProceduralMeshComponent.h>

// Sets default values
AZoneBuilder::AZoneBuilder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	mesh = CreateDefaultSubobject<UProceduralMeshComponent>("ZonesMesh");
}

// Called when the game starts or when spawned
void AZoneBuilder::BeginPlay()
{
	Super::BeginPlay();
	
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

void AZoneBuilder::GenerateZone(TArray<FVector> vertices, TArray<FVector> normals) 
{
	Zone* newZone = new Zone(vertices, normals, zones.Num() + 1);
	zones.Add(newZone);

	CalculateMesh();
}

void AZoneBuilder::CalculateMesh() 
{
	meshVertices.Empty();
	meshIndices.Empty();

	for (Zone* zone : zones) 
	{
		TArray<int> zoneIndices = zone->GetIndices();
		for (int32 i = 0; i < zoneIndices.Num(); i++)
			meshIndices.Add(zoneIndices[i] + meshVertices.Num());

		meshVertices.Append(zone->GetVertices());
	}

	mesh->CreateMeshSection(0, meshVertices, meshIndices, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), true);
}



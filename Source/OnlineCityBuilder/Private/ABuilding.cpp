#include "ABuilding.h"

#include <ProceduralMeshComponent.h>
#include <DrawDebugHelpers.h>



// Sets default values
ABuilding::ABuilding()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	buildingMesh = CreateDefaultSubobject<UProceduralMeshComponent>("BuildingMesh");

}

// Called when the game starts or when spawned
void ABuilding::BeginPlay()
{
	Super::BeginPlay();

	storeysRange = FIntVector2(3, 10);

	maxDimensions = FVector2D(2000, 2000);
	minDimensions = FVector2D(300, 600);
}

// Called every frame
void ABuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Building(DeltaTime);
}

void ABuilding::StartBuilding(TArray<FVector> area) 
{
	storeys = FMath::RandRange(storeysRange.X, storeysRange.Y);
	//storeys = 1;
	float width = FMath::RandRange(minDimensions.X, maxDimensions.X);
	float depth = FMath::RandRange(minDimensions.Y, maxDimensions.Y);
	FVector2D dimensions = FVector2D(width * 0.5, depth * 0.5);
	float height = storeys * MAX_STORY_HEIGHT;

	FVector areaCenter;
	for (FVector& lotVert : area) 
	{
		//DrawDebugSphere(GetWorld(), lotVert, 300, 4, FColor::Yellow, true);
		areaCenter += lotVert;
	}
	areaCenter /= area.Num();

	SetActorLocation(areaCenter);

	DrawDebugSphere(GetWorld(), GetActorLocation(), 300, 4, FColor::Cyan, true);

	CalculateMesh(area, FVector(dimensions.X, dimensions.Y, height));
}

void ABuilding::Building(float dt) 
{
	buildingProgress += dt;

	if (buildingProgress >= 1) 
	{
		PrimaryActorTick.bCanEverTick = false;
		onBuildingConstructed.ExecuteIfBound();
	}
}

void ABuilding::CalculateMesh(TArray<FVector> area, FVector dimensions)
{
	FVector bottomLeft, bottomRight, topLeft, topRight;
	FVector2D sqrArea = FVector2D(dimensions.X, dimensions.Y);

	bottomLeft = area[0] - GetActorLocation();
	bottomLeft.Normalize();
	bottomLeft *= sqrArea.Length();
	bottomLeft += GetActorLocation();

	bottomRight = area[1] - GetActorLocation();
	bottomRight.Normalize();
	bottomRight *= sqrArea.Length();
	bottomRight += GetActorLocation();

	topLeft = area[2] - GetActorLocation();
	topLeft.Normalize();
	topLeft *= sqrArea.Length();
	topLeft += GetActorLocation();

	topRight = area[3] - GetActorLocation();
	topRight.Normalize();
	topRight *= sqrArea.Length();
	topRight += GetActorLocation();

	vertices.Add(bottomLeft);
	vertices.Add(bottomRight);
	vertices.Add(topRight);
	vertices.Add(topLeft);

	DrawDebugSphere(GetWorld(), bottomLeft, 100, 4, FColor::Black, true);
	DrawDebugSphere(GetWorld(), bottomRight, 100, 4, FColor::Black, true);
	DrawDebugSphere(GetWorld(), topLeft, 100, 4, FColor::Black, true);
	DrawDebugSphere(GetWorld(), topRight, 100, 4, FColor::Black, true);

	//ceiling verts
	vertices.Add(bottomLeft + FVector::UpVector * dimensions.Z);
	vertices.Add(bottomRight + FVector::UpVector * dimensions.Z);
	vertices.Add(topRight + FVector::UpVector * dimensions.Z);
	vertices.Add(topLeft + FVector::UpVector * dimensions.Z);

	DrawDebugSphere(GetWorld(), vertices[4], 100, 4, FColor::Black, true);
	DrawDebugSphere(GetWorld(), vertices[5], 100, 4, FColor::Black, true);
	DrawDebugSphere(GetWorld(), vertices[6], 100, 4, FColor::Black, true);
	DrawDebugSphere(GetWorld(), vertices[7], 100, 4, FColor::Black, true);

	TArray<FVector> buildingVerts;
	TArray<int32> buildingIndices;

	const FPositionVertexBuffer& windowVB = viewData->windowMesh->GetRenderData()->GetCurrentFirstLOD(0)->VertexBuffers.PositionVertexBuffer;
	const FRawStaticIndexBuffer& windowIB = viewData->windowMesh->GetRenderData()->GetCurrentFirstLOD(0)->IndexBuffer;

	for (int16 storey = 0; storey < storeys; storey++)
	{
		for (int16 side = 0; side < 4; side++)
		{
			FVector dir = (side != 3 ? vertices[side + 1] : vertices[0]) - vertices[side];
			int32 windowsAmount = FMath::Floor(dir.Length() / 100);
			dir.Normalize();

			FQuat quat = FQuat(FVector::UpVector, 270 * side);
			FRotator rot = FRotator(quat);

			for (int32 j = 0; j < windowsAmount; j++)
			{
				FTransform transform = FTransform(rot, vertices[side] + dir * 100 * j + FVector::UpVector * MAX_STORY_HEIGHT * storey, FVector::One());
				TArray<FVector> transformedWindowVerts;
				for (uint32 k = 0; k < windowVB.GetNumVertices(); k++)
				{
					FVector localMeshVert = (FVector)windowVB.VertexPosition(k);
					transformedWindowVerts.Add(transform.TransformPosition(localMeshVert));
					DrawDebugSphere(GetWorld(), transformedWindowVerts[k], 10, 4, FColor::Blue, true);
				}

				TArray<uint32> shiftedIndices;
				for (int32 k = 0; k < windowIB.GetNumIndices(); k++)
				{
					shiftedIndices.Add(windowIB.GetIndex(k) + buildingIndices.Num());
				}

				buildingVerts.Append(transformedWindowVerts);
				buildingIndices.Append(shiftedIndices);
			}
		}
	}

	buildingMesh->CreateMeshSection(0, buildingVerts, buildingIndices, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), false);
	buildingMesh->SetMaterial(0, viewData->mat);
}
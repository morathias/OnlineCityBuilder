#include "ABuilding.h"

#include <ProceduralMeshComponent.h>
#include <DrawDebugHelpers.h>

// Sets default values
ABuilding::ABuilding()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABuilding::BeginPlay()
{
	Super::BeginPlay();

	storeysRange = FIntVector2(10, 50);

	maxDimensions = FVector2D(2000, 2000);
	minDimensions = FVector2D(300, 600);

	buildingMesh = NewObject<UProceduralMeshComponent>(this, "BuildingMesh");
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

	//DrawDebugSphere(GetWorld(), areaCenter, 300, 4, FColor::Cyan, true);

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

	bottomRight = area[2] - GetActorLocation();
	bottomRight.Normalize();
	bottomRight *= sqrArea.Length();

	topLeft = area[1] - GetActorLocation();
	topLeft.Normalize();
	topLeft *= sqrArea.Length();

	topRight = area[3] - GetActorLocation();
	topRight.Normalize();
	topRight *= sqrArea.Length();

	vertices.Add(bottomLeft);
	vertices.Add(bottomRight);
	vertices.Add(topLeft);
	vertices.Add(topRight);

	/*DrawDebugSphere(GetWorld(), bottomLeft, 100, 4, FColor::Red, true);
	DrawDebugSphere(GetWorld(), bottomRight, 100, 4, FColor::Red, true);
	DrawDebugSphere(GetWorld(), topLeft, 100, 4, FColor::Red, true);
	DrawDebugSphere(GetWorld(), topRight, 100, 4, FColor::Red, true);*/

	//ceiling verts
	vertices.Add(bottomLeft + FVector::UpVector * dimensions.Z);
	vertices.Add(bottomRight + FVector::UpVector * dimensions.Z);
	vertices.Add(topLeft + FVector::UpVector * dimensions.Z);
	vertices.Add(topRight + FVector::UpVector * dimensions.Z);

	TArray<FVector> buildingVerts;
	TArray<int32> buildingIndices;

	const FPositionVertexBuffer& windowVB = viewData->windowMesh->GetRenderData()->GetCurrentFirstLOD(0)->VertexBuffers.PositionVertexBuffer;
	const FRawStaticIndexBuffer& windowIB = viewData->windowMesh->GetRenderData()->GetCurrentFirstLOD(0)->IndexBuffer;

	FRotator rotator = FRotator(0, 0, 90);
	FVector bottomRightToLeft = bottomLeft - bottomRight;
	float distBottomRightToLeft = bottomRightToLeft.Length();
	bottomRightToLeft.Normalize();
	bottomRightToLeft = rotator.RotateVector(bottomRightToLeft);

	int32 windowsAmount = FMath::Floor(distBottomRightToLeft / 100);
	for (int32 i = 0; i < 2; i++)
	{
		for (int32 side = 0; side < 4; side++)
		{
			FQuat quat = FQuat(bottomRightToLeft, side * 90);
			FRotator rot = FRotator(quat);
			
			for (int32 j = 0; j < windowsAmount; j++)
			{
				FTransform transform = FTransform(rot, (bottomLeft + bottomRightToLeft * 100) + bottomRightToLeft * 100 * i + FVector::UpVector * MAX_STORY_HEIGHT * j, FVector::One() * 100);
				TArray<FVector> transformedWindowVerts;
				for (uint32 k = 0; k < windowVB.GetNumVertices(); k++)
				{
					transformedWindowVerts.Add(transform.TransformPosition((FVector)windowVB.VertexPosition(k)));
					//transformedWindowVerts.Add((FVector)windowVB.VertexPosition(k));
					//DrawDebugSphere(GetWorld(), transformedWindowVerts[k], 100, 4, FColor::Blue, true);
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
}

TArray<FVector> ABuilding::GetVerticesFromStaticMesh(const UStaticMesh& staticMesh)
{
	const FPositionVertexBuffer& meshVB = staticMesh.GetRenderData()->GetCurrentFirstLOD(0)->VertexBuffers.PositionVertexBuffer;

	FVector3f* rawVerticesData = (FVector3f*)(meshVB.GetVertexData());
	TArray<FVector3f> rawVerts;
	FMemory::Memcpy(rawVerticesData, rawVerticesData, meshVB.GetNumVertices());

	TArray<FVector> verts;
	verts.Append(rawVerts);

	return verts;
}

TArray<uint32> ABuilding::GetIndecisFromStaticMesh(const UStaticMesh& staticMesh)
{
	const FRawStaticIndexBuffer& meshIB = staticMesh.GetRenderData()->GetCurrentFirstLOD(0)->IndexBuffer;

	TArray<uint32> indices;
	meshIB.GetCopy(indices);

	return indices;
}
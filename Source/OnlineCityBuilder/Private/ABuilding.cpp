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

	//DrawDebugSphere(GetWorld(), GetActorLocation(), 300, 4, FColor::Cyan, true);

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

float ABuilding::AngleBetweenVectors(FVector Vec1, FVector Vec2)
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

void ABuilding::CalculateMesh(TArray<FVector> area, FVector dimensions)
{
	FVector bottomLeft, bottomRight, topLeft, topRight;
	FVector2D sqrArea = FVector2D(dimensions.X, dimensions.Y);

	bottomLeft = area[0] - GetActorLocation();
	bottomLeft.Normalize();
	bottomLeft *= sqrArea.Length();
	//bottomLeft += GetActorLocation();

	bottomRight = area[1] - GetActorLocation();
	bottomRight.Normalize();
	bottomRight *= sqrArea.Length();
	//bottomRight += GetActorLocation();

	topLeft = area[2] - GetActorLocation();
	topLeft.Normalize();
	topLeft *= sqrArea.Length();
	//topLeft += GetActorLocation();

	topRight = area[3] - GetActorLocation();
	topRight.Normalize();
	topRight *= sqrArea.Length();
	//topRight += GetActorLocation();

	FVector streetDir = bottomRight - bottomLeft;
	streetDir.Normalize();

	FVector buildingDir = bottomLeft - topLeft;
	buildingDir.Normalize();

	SetActorRotation(buildingDir.Rotation());

	vertices.Add(bottomLeft);
	vertices.Add(bottomRight);
	vertices.Add(topRight);
	vertices.Add(topLeft);

	/*DrawDebugSphere(GetWorld(), bottomLeft, 100, 4, FColor::Black, true);
	DrawDebugSphere(GetWorld(), bottomRight, 100, 4, FColor::Black, true);
	DrawDebugSphere(GetWorld(), topLeft, 100, 4, FColor::Black, true);
	DrawDebugSphere(GetWorld(), topRight, 100, 4, FColor::Black, true);*/

	//ceiling verts
	vertices.Add(bottomLeft + FVector::UpVector * dimensions.Z);
	vertices.Add(bottomRight + FVector::UpVector * dimensions.Z);
	vertices.Add(topRight + FVector::UpVector * dimensions.Z);
	vertices.Add(topLeft + FVector::UpVector * dimensions.Z);

	/*DrawDebugSphere(GetWorld(), vertices[4], 100, 4, FColor::Black, true);
	DrawDebugSphere(GetWorld(), vertices[5], 100, 4, FColor::Black, true);
	DrawDebugSphere(GetWorld(), vertices[6], 100, 4, FColor::Black, true);
	DrawDebugSphere(GetWorld(), vertices[7], 100, 4, FColor::Black, true);*/

	TArray<FVector> buildingVerts;
	TArray<int32> buildingIndices;
	TArray<FVector> buildingNormals;

	const FPositionVertexBuffer& windowVB = viewData->windowMesh->GetRenderData()->GetCurrentFirstLOD(0)->VertexBuffers.PositionVertexBuffer;
	const FRawStaticIndexBuffer& windowIB = viewData->windowMesh->GetRenderData()->GetCurrentFirstLOD(0)->IndexBuffer;
	const FStaticMeshVertexBuffer& windowSVB = viewData->windowMesh->GetRenderData()->GetCurrentFirstLOD(0)->VertexBuffers.StaticMeshVertexBuffer;

	for (int8 i = 0; i < vertices.Num(); i++)
	{
		vertices[i] = GetActorTransform().TransformVector(vertices[i]);
		DrawDebugSphere(GetWorld(), vertices[i] + GetActorLocation(), 20, 4, FColor::Red, true);
	}

	/*DrawDebugDirectionalArrow(GetWorld(), vertices[0] + GetActorLocation(), vertices[1] + GetActorLocation(), 50, FColor::Black, true);
	DrawDebugDirectionalArrow(GetWorld(), vertices[1] + GetActorLocation(), vertices[2] + GetActorLocation(), 50, FColor::Black, true);
	DrawDebugDirectionalArrow(GetWorld(), vertices[2] + GetActorLocation(), vertices[3] + GetActorLocation(), 50, FColor::Black, true);
	DrawDebugDirectionalArrow(GetWorld(), vertices[3] + GetActorLocation(), vertices[0] + GetActorLocation(), 50, FColor::Black, true);

	DrawDebugString(GetWorld(), vertices[0] + GetActorLocation(), TEXT("bottom left"), (AActor*)0, FColor::Black);
	DrawDebugString(GetWorld(), vertices[1] + GetActorLocation(), TEXT("bottom right"), (AActor*)0, FColor::Black);
	DrawDebugString(GetWorld(), vertices[2] + GetActorLocation(), TEXT("top right"), (AActor*)0, FColor::Black);
	DrawDebugString(GetWorld(), vertices[3] + GetActorLocation(), TEXT("top left"), (AActor*)0, FColor::Black);*/

	for (int16 storey = 0; storey < storeys; storey++)
	{
		for (int16 side = 0; side < 4; side++)
		{
			FVector dir = (side != 3 ? vertices[side + 1] : vertices[0]) - vertices[side];

			int32 windowsAmount = FMath::CeilToInt32(dir.Length() / 100);
			float wallSize = windowsAmount * 100;
			float scaleAmount = dir.Length() / wallSize;
 			dir.Normalize();

			//FRotator rot = FRotator(dir.RotateAngleAxis(90 * FVector::DotProduct(GetActorRightVector(), streetDir), FVector::UpVector).Rotation());
			FRotator rot = FRotator(dir.Rotation());

			for (int16 j = 0; j < windowsAmount; j++)
			{
				FTransform transform = FTransform(rot, vertices[side] + dir * 100 * scaleAmount * j + FVector::UpVector * MAX_STORY_HEIGHT * storey, FVector(1, scaleAmount, 1));
				for (uint32 k = 0; k < windowVB.GetNumVertices(); k++)
				{
					FVector localMeshVert = (FVector)windowVB.VertexPosition(k);
					buildingVerts.Add(transform.TransformPosition(localMeshVert));
					buildingNormals.Add(transform.TransformVector((FVector)windowSVB.VertexTangentZ(k)));
					//DrawDebugSphere(GetWorld(), transform.TransformPosition(localMeshVert) + GetActorLocation(), 10, 4, FColor::Blue, true);
					//DrawDebugString(GetWorld(), transform.TransformPosition(localMeshVert) + GetActorLocation(), TEXT(""+ FString::FromInt(buildingVerts.Num() + k)), (AActor*)0, FColor::Black);
				}

				TArray<uint32> shiftedIndices;
				for (int16 k = 0; k < windowIB.GetNumIndices(); k++)
				{
					shiftedIndices.Add(windowIB.GetIndex(k) + FMath::Clamp(buildingIndices.Num() - 2 * (buildingIndices.Num() / 6), 0, buildingIndices.Num()));
				}

				buildingIndices.Append(shiftedIndices);
			}
		}
	}

	const FPositionVertexBuffer& ceilingBorderVB = viewData->ceilingBorderMesh->GetRenderData()->GetCurrentFirstLOD(0)->VertexBuffers.PositionVertexBuffer;
	const FRawStaticIndexBuffer& ceilingBorderIB = viewData->ceilingBorderMesh->GetRenderData()->GetCurrentFirstLOD(0)->IndexBuffer;
	const FStaticMeshVertexBuffer& ceilingBorderSVB = viewData->ceilingBorderMesh->GetRenderData()->GetCurrentFirstLOD(0)->VertexBuffers.StaticMeshVertexBuffer;

	for (int16 side = 4; side < 8; side++)
	{
		FVector dir = (side != 7 ? vertices[side + 1] : vertices[4]) - vertices[side];

		int32 bordersAmount = FMath::CeilToInt32(dir.Length() / 100);
		float borderSize = bordersAmount * 100;
		float scaleAmount = dir.Length() / borderSize;
		dir.Normalize();

		FRotator rot = FRotator(dir.Rotation());

		int sideVertsAmount = buildingVerts.Num();

		for (int16 j = 0; j < bordersAmount; j++)
		{
			TArray<uint32> shiftedIndices;
			for (int16 k = 0; k < ceilingBorderIB.GetNumIndices(); k++)
			{
				shiftedIndices.Add(ceilingBorderIB.GetIndex(k) + ceilingBorderVB.GetNumVertices() * j + sideVertsAmount);
			}
			buildingIndices.Append(shiftedIndices);
			
			FTransform transform = FTransform(rot, vertices[side] + dir * 100 * scaleAmount * j, FVector(1, scaleAmount, 1));
			for (uint32 k = 0; k < ceilingBorderVB.GetNumVertices(); k++)
			{
				FVector localMeshVert = (FVector)ceilingBorderVB.VertexPosition(k);
				buildingVerts.Add(transform.TransformPosition(localMeshVert));
				buildingNormals.Add(transform.TransformVector((FVector)ceilingBorderSVB.VertexTangentZ(k)));
				//DrawDebugSphere(GetWorld(), transform.TransformPosition(localMeshVert) + GetActorLocation(), 10, 4, FColor::Blue, true);
				//DrawDebugString(GetWorld(), transform.TransformPosition(localMeshVert) + GetActorLocation(), TEXT(""+ FString::FromInt(buildingVerts.Num() + k)), (AActor*)0, FColor::Black);
			}

			

		}
	}

	buildingMesh->CreateMeshSection(0, buildingVerts, buildingIndices, buildingNormals, TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), false);
	buildingMesh->SetMaterial(0, viewData->mat);
}


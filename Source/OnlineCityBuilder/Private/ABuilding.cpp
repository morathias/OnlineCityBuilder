#include "ABuilding.h"

#include <ProceduralMeshComponent.h>
#include <DrawDebugHelpers.h>
#include <RealtimeMeshComponent.h>
#include <RealtimeMeshSimple.h>


// Sets default values
ABuilding::ABuilding()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//buildingMesh = CreateDefaultSubobject<UProceduralMeshComponent>("BuildingMesh");
	realtimeMesh = CreateDefaultSubobject<URealtimeMeshComponent>("BuildingMesh");
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

	//SetActorRotation(buildingDir.Rotation());

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

	/*DrawDebugDirectionalArrow(GetWorld(), vertices[0] + GetActorLocation(), vertices[1] + GetActorLocation(), 50, FColor::Black, true);
	DrawDebugDirectionalArrow(GetWorld(), vertices[1] + GetActorLocation(), vertices[2] + GetActorLocation(), 50, FColor::Black, true);
	DrawDebugDirectionalArrow(GetWorld(), vertices[2] + GetActorLocation(), vertices[3] + GetActorLocation(), 50, FColor::Black, true);
	DrawDebugDirectionalArrow(GetWorld(), vertices[3] + GetActorLocation(), vertices[0] + GetActorLocation(), 50, FColor::Black, true);*/

	//DrawDebugString(GetWorld(), vertices[0] + GetActorLocation(), TEXT("bottom left"), (AActor*)0, FColor::Black);
	//DrawDebugString(GetWorld(), vertices[1] + GetActorLocation(), TEXT("bottom right"), (AActor*)0, FColor::Black);
	//DrawDebugString(GetWorld(), vertices[2] + GetActorLocation(), TEXT("top right"), (AActor*)0, FColor::Black);
	//DrawDebugString(GetWorld(), vertices[3] + GetActorLocation(), TEXT("top left"), (AActor*)0, FColor::Black);

	TArray<FVector> wallCornerBuildArea = { vertices[0], vertices[1], vertices[2], vertices[3] };
	BuildMesh(*(viewData->cornerMesh), wallCornerBuildArea, storeys, buildingVerts, buildingIndices, buildingNormals, 1);
	BuildMesh(*(viewData->windowMesh), wallCornerBuildArea, storeys, buildingVerts, buildingIndices, buildingNormals, -1, true);

	TArray<FVector> ceilingCornerBuildArea = { vertices[4], vertices[5], vertices[6], vertices[7] };
	BuildMesh(*(viewData->ceilingCornerMesh), ceilingCornerBuildArea, 1, buildingVerts, buildingIndices, buildingNormals, 1);
	BuildMesh(*(viewData->ceilingBorderMesh), ceilingCornerBuildArea, 1, buildingVerts, buildingIndices, buildingNormals, -1, true);

	TArray<FVector> ceilingFloorBuildArea = { 
											  vertices[4] + streetDir * (viewData->cornerMesh->GetBounds().GetBox().GetSize().X) + buildingDir * (viewData->cornerMesh->GetBounds().GetBox().GetSize().X) * -1,
											  vertices[5] + (streetDir * (viewData->cornerMesh->GetBounds().GetBox().GetSize().X) * -1 + buildingDir * (viewData->cornerMesh->GetBounds().GetBox().GetSize().X) * -1),
											  vertices[6] + (streetDir * (viewData->cornerMesh->GetBounds().GetBox().GetSize().X) * -1 + buildingDir * (viewData->cornerMesh->GetBounds().GetBox().GetSize().X)),
											  vertices[7] + streetDir * (viewData->cornerMesh->GetBounds().GetBox().GetSize().X) + buildingDir * (viewData->cornerMesh->GetBounds().GetBox().GetSize().X)
											};
	BuildMeshGrid(*(viewData->ceilingFloorMesh), ceilingFloorBuildArea, buildingVerts, buildingIndices, buildingNormals);

	URealtimeMeshSimple* generatedMesh = realtimeMesh->InitializeRealtimeMesh<URealtimeMeshSimple>();
	FRealtimeMeshSimpleMeshData LODMeshData;
	LODMeshData.Positions = buildingVerts;
	LODMeshData.Triangles = buildingIndices;
	LODMeshData.Normals = buildingNormals;

	FRealtimeMeshLODKey LODKey = FRealtimeMeshLODKey(0);
	generatedMesh->CreateSectionGroup(FRealtimeMeshSectionGroupKey::CreateUnique(LODKey), LODMeshData);
	generatedMesh->SetupMaterialSlot(0, "fafa", viewData->mat);

	/*generatedMesh->CreateSection(FRealtimeMeshSectionKey::Create(FRealtimeMeshSectionGroupKey::CreateUnique(FRealtimeMeshLODKey::FRealtimeMeshLODKey(0))),
																FRealtimeMeshSectionConfig::FRealtimeMeshSectionConfig(),*/

	//buildingMesh->CreateMeshSection(0, buildingVerts, buildingIndices, buildingNormals, TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), false);
	//buildingMesh->SetMaterial(0, viewData->mat);
}

void ABuilding::BuildMesh(const UStaticMesh& mesh, TArray<FVector> buildArea, uint8 inStoreys, TArray<FVector>& currentBuildingVerts, TArray<int32>& currentBuildingIndices, TArray<FVector>& currentBuildingNormals, int amount, bool removeCorners)
{
	const FPositionVertexBuffer& meshVB = mesh.GetRenderData()->GetCurrentFirstLOD(0)->VertexBuffers.PositionVertexBuffer;
	const FRawStaticIndexBuffer& meshIB = mesh.GetRenderData()->GetCurrentFirstLOD(0)->IndexBuffer;
	const FStaticMeshVertexBuffer& meshSVB = mesh.GetRenderData()->GetCurrentFirstLOD(0)->VertexBuffers.StaticMeshVertexBuffer;

	float meshWidth = mesh.GetBounds().GetBox().GetSize().X;

	for (uint8 storey = 0; storey < inStoreys; storey++)
	{
		for (int16 side = 0; side < 4; side++)
		{
			FVector dir = (side != 3 ? buildArea[side + 1] : buildArea[0]) - buildArea[side];

			int32 meshAmount = FMath::CeilToInt32(dir.Length() / meshWidth);
			float wallSize = meshAmount * meshWidth;
			float scaleAmount = dir.Length() / wallSize;
			dir.Normalize();

			if (amount != -1)
			{
				meshAmount = amount;
				scaleAmount = 1;
			}

			FRotator rot = FRotator(dir.Rotation());

			int sideVertsAmount = currentBuildingVerts.Num();

			for (int16 j = 0; j < meshAmount; j++)
			{			
				if (removeCorners && (j == 0 || j == meshAmount - 1)) 
				{
					continue;
				}

				TArray<uint32> shiftedIndices;
				for (int16 k = 0; k < meshIB.GetNumIndices(); k++)
				{
					shiftedIndices.Add(meshIB.GetIndex(k) + meshVB.GetNumVertices() * (!removeCorners ? j : j - 1) + sideVertsAmount);
				}
				currentBuildingIndices.Append(shiftedIndices);

				FTransform transform = FTransform(rot, buildArea[side] + dir * meshWidth * scaleAmount * j + FVector::UpVector * MAX_STORY_HEIGHT * storey, FVector(1, scaleAmount, 1));
				for (uint32 k = 0; k < meshVB.GetNumVertices(); k++)
				{
					FVector localMeshVert = (FVector)meshVB.VertexPosition(k);
					currentBuildingVerts.Add(transform.TransformPosition(localMeshVert));
					currentBuildingNormals.Add(transform.TransformVector((FVector)meshSVB.VertexTangentZ(k)));
					//DrawDebugSphere(GetWorld(), transform.TransformPosition(localMeshVert) + GetActorLocation(), 10, 4, FColor::Blue, true);
					//DrawDebugString(GetWorld(), transform.TransformPosition(localMeshVert) + GetActorLocation(), TEXT(""+ FString::FromInt(buildingVerts.Num() + k)), (AActor*)0, FColor::Black);
				}
			}
		}
	}
}

void ABuilding::BuildMeshGrid(const UStaticMesh& mesh, TArray<FVector> buildArea, TArray<FVector>& currentBuildingVerts, TArray<int32>& currentBuildingIndices, TArray<FVector>& currentBuildingNormals)
{
	const FPositionVertexBuffer& meshVB = mesh.GetRenderData()->GetCurrentFirstLOD(0)->VertexBuffers.PositionVertexBuffer;
	const FRawStaticIndexBuffer& meshIB = mesh.GetRenderData()->GetCurrentFirstLOD(0)->IndexBuffer;
	const FStaticMeshVertexBuffer& meshSVB = mesh.GetRenderData()->GetCurrentFirstLOD(0)->VertexBuffers.StaticMeshVertexBuffer;

	FVector rowDir = (buildArea[1] - buildArea[0]);
	FVector columnDir = (buildArea[3] - buildArea[0]);

	float rowSize = rowDir.Length();
	float columnSize = columnDir.Length();
	int16 rowAmount = FMath::CeilToInt(columnDir.Length() / 100);
	int16 columnAmount = FMath::CeilToInt32(rowDir.Length() / 100);
	rowDir.Normalize();
	columnDir.Normalize();

	for (int16 row = 0; row < rowAmount; row++)
	{
		float scaleAmountColumn = rowSize / (columnAmount * 100);
		float scaleAmountRow = columnSize / (rowAmount * 100);

		FRotator rot = FRotator(rowDir.Rotation());

		int sideVertsAmount = currentBuildingVerts.Num();

		for (int16 column = 0; column < columnAmount; column++)
		{
			TArray<uint32> shiftedIndices;
			for (int16 k = 0; k < meshIB.GetNumIndices(); k++)
			{
				shiftedIndices.Add(meshIB.GetIndex(k) + meshVB.GetNumVertices() * column + sideVertsAmount);
			}
			currentBuildingIndices.Append(shiftedIndices);

			FTransform transform = FTransform(rot, buildArea[0] + rowDir * 100 * scaleAmountColumn * column + columnDir * 100 * scaleAmountRow * row, FVector(scaleAmountColumn, scaleAmountRow, 1));
			for (uint32 k = 0; k < meshVB.GetNumVertices(); k++)
			{
				FVector localMeshVert = (FVector)meshVB.VertexPosition(k);
				currentBuildingVerts.Add(transform.TransformPosition(localMeshVert));
				currentBuildingNormals.Add(transform.TransformVector((FVector)meshSVB.VertexTangentZ(k)));
				//DrawDebugSphere(GetWorld(), transform.TransformPosition(localMeshVert) + GetActorLocation(), 10, 4, FColor::Blue, true);
				//DrawDebugString(GetWorld(), transform.TransformPosition(localMeshVert) + GetActorLocation(), TEXT(""+ FString::FromInt(buildingVerts.Num() + k)), (AActor*)0, FColor::Black);
			}
		}
	}
}


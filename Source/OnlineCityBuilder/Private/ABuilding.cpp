#include "ABuilding.h"

#include <DrawDebugHelpers.h>
#include <RealtimeMeshComponent.h>
#include <RealtimeMeshSimple.h>
#include <Mesh/RealtimeMeshAlgo.h>

typedef TRealtimeMeshBuilderLocal<uint16, FPackedNormal, FVector2DHalf, 1, uint16> MeshBuilder;

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

	bottomRight = area[1] - GetActorLocation();
	bottomRight.Normalize();
	bottomRight *= sqrArea.Length();

	topLeft = area[2] - GetActorLocation();
	topLeft.Normalize();
	topLeft *= sqrArea.Length();

	topRight = area[3] - GetActorLocation();
	topRight.Normalize();
	topRight *= sqrArea.Length();

	FVector streetDir = bottomRight - bottomLeft;
	streetDir.Normalize();

	FVector buildingDir = bottomLeft - topLeft;
	buildingDir.Normalize();

	vertices.Add(bottomLeft);
	vertices.Add(bottomRight);
	vertices.Add(topRight);
	vertices.Add(topLeft);

	//ceiling verts
	vertices.Add(bottomLeft + FVector::UpVector * dimensions.Z);
	vertices.Add(bottomRight + FVector::UpVector * dimensions.Z);
	vertices.Add(topRight + FVector::UpVector * dimensions.Z);
	vertices.Add(topLeft + FVector::UpVector * dimensions.Z);

	FRealtimeMeshStreamSet streamSet;
	TArray<FVector> wallCornerBuildArea = { vertices[0], vertices[1], vertices[2], vertices[3] };
	BuildMesh(*(viewData->cornerMesh), wallCornerBuildArea, storeys, streamSet, 1);
	BuildMesh(*(viewData->windowMesh), wallCornerBuildArea, storeys, streamSet, -1, true);

	TArray<FVector> ceilingCornerBuildArea = { vertices[4], vertices[5], vertices[6], vertices[7] };
	BuildMesh(*(viewData->ceilingCornerMesh), ceilingCornerBuildArea, 1, streamSet, 1);
	BuildMesh(*(viewData->ceilingBorderMesh), ceilingCornerBuildArea, 1, streamSet, -1, true);

	TArray<FVector> ceilingFloorBuildArea = { 
											  vertices[4] + streetDir * (viewData->cornerMesh->GetBounds().GetBox().GetSize().X) + buildingDir * (viewData->cornerMesh->GetBounds().GetBox().GetSize().X) * -1,
											  vertices[5] + (streetDir * (viewData->cornerMesh->GetBounds().GetBox().GetSize().X) * -1 + buildingDir * (viewData->cornerMesh->GetBounds().GetBox().GetSize().X) * -1),
											  vertices[6] + (streetDir * (viewData->cornerMesh->GetBounds().GetBox().GetSize().X) * -1 + buildingDir * (viewData->cornerMesh->GetBounds().GetBox().GetSize().X)),
											  vertices[7] + streetDir * (viewData->cornerMesh->GetBounds().GetBox().GetSize().X) + buildingDir * (viewData->cornerMesh->GetBounds().GetBox().GetSize().X)
											};
	BuildMeshGrid(*(viewData->ceilingFloorMesh), ceilingCornerBuildArea, streamSet);

	URealtimeMeshSimple* generatedMesh = realtimeMesh->InitializeRealtimeMesh<URealtimeMeshSimple>();

	FRealtimeMeshLODKey LODKey = FRealtimeMeshLODKey(0);
	FRealtimeMeshSectionGroupKey groupKey = FRealtimeMeshSectionGroupKey::CreateUnique(LODKey);

	for (int8 i = 0; i < viewData->mats.Num(); i++)
	{
		generatedMesh->SetupMaterialSlot(i, FName(viewData->mats[i]->GetName()), viewData->mats[i]);
	}

	RealtimeMeshAlgo::OrganizeTrianglesByPolygonGroup(streamSet, FRealtimeMeshStreams::Triangles, FRealtimeMeshStreams::PolyGroups);
	generatedMesh->CreateSectionGroup(groupKey, streamSet, true);
}

void ABuilding::BuildMesh(const UStaticMesh& mesh, TArray<FVector> buildArea, uint8 inStoreys, FRealtimeMeshStreamSet& streamSet, int amount, bool removeCorners)
{
	const FPositionVertexBuffer& meshVB = mesh.GetRenderData()->GetCurrentFirstLOD(0)->VertexBuffers.PositionVertexBuffer;
	const FRawStaticIndexBuffer& meshIB = mesh.GetRenderData()->GetCurrentFirstLOD(0)->IndexBuffer;
	const FStaticMeshVertexBuffer& meshSVB = mesh.GetRenderData()->GetCurrentFirstLOD(0)->VertexBuffers.StaticMeshVertexBuffer;	

	MeshBuilder builder(streamSet);
	builder.EnablePolyGroups();
	builder.EnableTangents();
	builder.EnableTexCoords();

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

			int sideVertsAmount = builder.NumVertices();

			for (int16 j = 0; j < meshAmount; j++)
			{			
				if (removeCorners && (j == 0 || j == meshAmount - 1)) 
				{
					continue;
				}

				FTransform transform = FTransform(rot, buildArea[side] + dir * meshWidth * scaleAmount * j + FVector::UpVector * MAX_STORY_HEIGHT * storey, FVector(1, scaleAmount, 1));
				for (uint32 k = 0; k < meshVB.GetNumVertices(); k++)
				{
					FVector localMeshVert = (FVector)meshVB.VertexPosition(k);
					FVector normals = transform.TransformVector((FVector)meshSVB.VertexTangentZ(k));
					normals.Normalize();
					FVector tangents = transform.TransformVector((FVector)meshSVB.VertexTangentX(k));
					tangents.Normalize();
					builder.AddVertex(FVector3f(transform.TransformPosition(localMeshVert)))
						.SetNormal(FVector3f(normals))
						.SetTangent(FVector3f(tangents))
						.SetTexCoord(0, meshSVB.GetVertexUV(k, 0));

					//DrawDebugSphere(GetWorld(), transform.TransformPosition(localMeshVert) + GetActorLocation(), 10, 4, FColor::Blue, true);
					//DrawDebugString(GetWorld(), transform.TransformPosition(localMeshVert) + GetActorLocation(), TEXT(""+ FString::FromInt(buildingVerts.Num() + k)), (AActor*)0, FColor::Black);
				}

				for (uint8 sectionIndex = 0; sectionIndex < mesh.GetRenderData()->GetCurrentFirstLOD(0)->Sections.Num(); sectionIndex++)
				{
					const FStaticMeshSection section = mesh.GetRenderData()->GetCurrentFirstLOD(0)->Sections[sectionIndex];
					for (uint16 k = section.FirstIndex; k < section.FirstIndex + section.NumTriangles * 3; k += 3)
					{
						uint32 index1 = meshIB.GetIndex(k) + (meshVB.GetNumVertices()) * (!removeCorners ? j : j - 1) + sideVertsAmount;
						uint32 index2 = meshIB.GetIndex(k + 1) + (meshVB.GetNumVertices()) * (!removeCorners ? j : j - 1) + sideVertsAmount;
						uint32 index3 = meshIB.GetIndex(k + 2) + (meshVB.GetNumVertices()) * (!removeCorners ? j : j - 1) + sideVertsAmount;

						builder.AddTriangle(index1, index2, index3, section.MaterialIndex);
					}
				}
			}
		}
	}
}

void ABuilding::BuildMeshGrid(const UStaticMesh& mesh, TArray<FVector> buildArea, FRealtimeMeshStreamSet& streamSet)
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

	MeshBuilder builder(streamSet);
	builder.EnablePolyGroups();
	builder.EnableTangents();
	builder.EnableTexCoords();

	for (int16 row = 0; row < rowAmount; row++)
	{
		float scaleAmountColumn = rowSize / (columnAmount * 100);
		float scaleAmountRow = columnSize / (rowAmount * 100);

		FRotator rot = FRotator(rowDir.Rotation());

		int sideVertsAmount = builder.NumVertices();

		for (int16 column = 0; column < columnAmount; column++)
		{
			FTransform transform = FTransform(rot, buildArea[0] + rowDir * 100 * scaleAmountColumn * column + columnDir * 100 * scaleAmountRow * row, FVector(scaleAmountColumn, scaleAmountRow, 1));
			for (uint32 k = 0; k < meshVB.GetNumVertices(); k++)
			{
				FVector localMeshVert = (FVector)meshVB.VertexPosition(k);

				builder.AddVertex(FVector3f(transform.TransformPosition(localMeshVert)))
					.SetNormal(FVector3f(transform.TransformVector((FVector)meshSVB.VertexTangentZ(k))))
					.SetTexCoord(0, meshSVB.GetVertexUV(k, 0));
				//DrawDebugSphere(GetWorld(), transform.TransformPosition(localMeshVert) + GetActorLocation(), 10, 4, FColor::Blue, true);
				//DrawDebugString(GetWorld(), transform.TransformPosition(localMeshVert) + GetActorLocation(), TEXT(""+ FString::FromInt(buildingVerts.Num() + k)), (AActor*)0, FColor::Black);
			}

			for (uint8 sectionIndex = 0; sectionIndex < mesh.GetRenderData()->GetCurrentFirstLOD(0)->Sections.Num(); sectionIndex++)
			{
				const FStaticMeshSection section = mesh.GetRenderData()->GetCurrentFirstLOD(0)->Sections[sectionIndex];
				for (uint16 k = section.FirstIndex; k < section.FirstIndex + section.NumTriangles * 3; k += 3)
				{
					uint32 index1 = meshIB.GetIndex(k) + (meshVB.GetNumVertices()) * column + sideVertsAmount;
					uint32 index2 = meshIB.GetIndex(k + 1) + (meshVB.GetNumVertices()) * column + sideVertsAmount;
					uint32 index3 = meshIB.GetIndex(k + 2) + (meshVB.GetNumVertices()) * column + sideVertsAmount;

					builder.AddTriangle(index1, index2, index3, section.MaterialIndex);
				}
			}
		}
	}
}


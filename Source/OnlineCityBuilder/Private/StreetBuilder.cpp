// Fill out your copyright notice in the Description page of Project Settings.


#include "StreetBuilder.h"
#include "DrawDebugHelpers.h"

#include "MyCityBuilderGameMode.h"
#include "MyCamera.h"

#include <ProceduralMeshComponent.h>

// Sets default values
AStreetBuilder::AStreetBuilder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	streetMesh = CreateDefaultSubobject<UProceduralMeshComponent>("StreetMesh");
	streetMeshPreview = CreateDefaultSubobject<UProceduralMeshComponent>("StreetMeshPreview");
}

// Called when the game starts or when spawned
void AStreetBuilder::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AStreetBuilder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(currentStreet != nullptr)
		PlacingEndRoad();

	AMyCityBuilderGameMode* gameMode = Cast<AMyCityBuilderGameMode>(GetWorld()->GetAuthGameMode());
	if (gameMode != nullptr && gameMode->isEditingRoad) 
	{
		DrawPreviewStreet(currentStreet != nullptr);
	}
	else
	{
		ClearPreviewStreet();
	}

	for (const Street* street: streets)
	{
		DrawDebugSphere(GetWorld(), street->startNode.position, street->startNode.radius, 16, FColor::Red);
		DrawDebugSphere(GetWorld(), street->endNode.position, street->endNode.radius, 16, FColor::Red);
	}
}

void AStreetBuilder::PlaceRoad(const FVector& position)
{
	if (currentStreet == nullptr)
	{
		currentStreet = new Street();

		currentStreet->startNode.position = position;
		currentStreet->startNode.type = StreetType::OneLane;

		currentStreet->endNode.position = position;
		currentStreet->endNode.type = StreetType::OneLane;
	}
	else
	{
		previousStreet = currentStreet;
		FinishRoad();

		currentStreet = new Street(previousStreet->endNode);
		currentStreet->endNode.position = position;

		currentStreet->connectedStreets.Add(previousStreet);
		previousStreet->connectedStreets.Add(currentStreet);
	}
}

void AStreetBuilder::PlacingEndRoad() 
{
	FVector position;
	if (Raycast(position))
	{
		currentStreet->endNode.position = position;
	}

	DrawDebugLine(GetWorld(), currentStreet->startNode.position, currentStreet->endNode.position, FColor::Blue);
	DrawDebugSphere(GetWorld(), currentStreet->startNode.position, 100, 12, FColor::Blue);
	DrawDebugSphere(GetWorld(), currentStreet->endNode.position, 100, 12, FColor::Blue);
}

void AStreetBuilder::FinishRoad() 
{
	streets.Add(currentStreet);
	currentStreet = nullptr;

	CalculateMesh();
}

void AStreetBuilder::CancelRoad() 
{
	if (currentStreet == nullptr) return; //no street to cancel

	delete currentStreet;
	currentStreet = nullptr;

	previousStreet = nullptr;
}

void AStreetBuilder::CalculateMesh() 
{
	triangles.Empty();
	vertices.Empty();
	uvs.Empty();
	streetMesh->ClearAllMeshSections();

	Street* lastPlacedStreet = streets[streets.Num() - 1];
	CalculateFacesForStreet(lastPlacedStreet, streets.Num() - 1);
		
	for (Street* connectedStreet : lastPlacedStreet->connectedStreets) 
	{
		bool connectedStreetCalculated = connectedStreet->vertices.Num() != 0;
		if (!connectedStreetCalculated)
			CalculateFacesForStreet(connectedStreet, streets.IndexOfByKey(connectedStreet));

		SeamCorrection(connectedStreet, lastPlacedStreet);
	}


	for (const Street* street : streets) 
	{
		vertices.Append(street->vertices);
		uvs.Append(street->uvs);
		triangles.Append(street->triangles);
	}

	streetMesh->CreateMeshSection(0, vertices, triangles, TArray<FVector>(), uvs, TArray<FColor>(), TArray<FProcMeshTangent>(), true);
}

void AStreetBuilder::SetMaterial(UMaterialInterface* roadMaterial)
{
	this->material = roadMaterial;
	this->materialPreview = roadMaterial;
	streetMesh->SetMaterial(0, material);
	streetMeshPreview->SetMaterial(0, material);
}

void AStreetBuilder::CalculateFacesForStreet(Street* street, int index /*= 0*/)
{
	FRotator rot(0, 90, 0);
	FVector currentStreetDir = street->endNode.position - street->startNode.position;
	currentStreetDir.Normalize();
	FVector currentStreetTangent = rot.RotateVector(currentStreetDir);

	FVector currentStreetVertDownLeft = street->startNode.position + currentStreetTangent * -1 * street->width * 0.5 + FVector::UpVector * 10;
	FVector currentStreetVertDownRight = street->startNode.position + currentStreetTangent * street->width * 0.5 + FVector::UpVector * 10;
	FVector currentStreetVertTopLeft = street->endNode.position + currentStreetTangent * -1 * street->width * 0.5 + FVector::UpVector * 10;
	FVector currentStreetVertTopRight = street->endNode.position + currentStreetTangent * street->width * 0.5 + FVector::UpVector * 10;

	street->vertices.Add(currentStreetVertDownLeft);
	street->vertices.Add(currentStreetVertDownRight);
	street->vertices.Add(currentStreetVertTopLeft);
	street->vertices.Add(currentStreetVertTopRight);

	street->uvs.Add(FVector2D(0, 0));
	street->uvs.Add(FVector2D(0, 1));
	street->uvs.Add(FVector2D(1, 0));
	street->uvs.Add(FVector2D(1, 1));

	//Triangle1
	street->triangles.Add(index * 4); //down left
	street->triangles.Add(index * 4 + 1); //down right
	street->triangles.Add(index * 4 + 2); // top left

	//Triangle2
	street->triangles.Add(index * 4 + 2);
	street->triangles.Add(index * 4 + 1);
	street->triangles.Add(index * 4 + 3); //top right
}

void AStreetBuilder::SeamCorrection(Street* bottomStreet, Street* topStreet) 
{
	if (bottomStreet->vertices[2] == topStreet->vertices[0]) //already corrected
		return;

	FVector bottomStreetVertDownLeft = bottomStreet->vertices[0];
	FVector bottomStreetVertDownRight = bottomStreet->vertices[1];
	FVector bottomStreetVertTopLeft = bottomStreet->vertices[2];
	FVector bottomStreetVertTopRight = bottomStreet->vertices[3];
	FVector bottomStreetDir = bottomStreet->endNode.position - bottomStreet->startNode.position;
	bottomStreetDir.Normalize();

	FVector topStreetVertDownLeft = topStreet->vertices[0];
	FVector topStreetVertDownRight = topStreet->vertices[1];
	FVector topStreetVertTopLeft = topStreet->vertices[2];
	FVector topStreetVertTopRight = topStreet->vertices[3];
	FVector topStreetDir = (topStreet->endNode.position - topStreet->startNode.position) * -1;
	topStreetDir.Normalize();

	FVector intersectionL;
	FMath::SegmentIntersection2D(bottomStreetVertDownLeft, bottomStreetVertTopLeft + bottomStreetDir * std::numeric_limits<float>::max(),
								 topStreetVertTopLeft, topStreetVertDownLeft + topStreetDir * std::numeric_limits<float>::max(),
								 intersectionL);

	FVector intersectionR;
	FMath::SegmentIntersection2D(bottomStreetVertDownRight, bottomStreetVertTopRight + bottomStreetDir * std::numeric_limits<float>::max(),
								 topStreetVertTopRight, topStreetVertDownRight + topStreetDir * std::numeric_limits<float>::max(),
								 intersectionR);

	bottomStreet->vertices[2] = intersectionL;
	bottomStreet->vertices[3] = intersectionR;

	topStreet->vertices[0] = intersectionL;
	topStreet->vertices[1] = intersectionR;
}

void AStreetBuilder::DrawPreviewStreet(bool isPlacing)
{
	if (!isPlacing) 
	{
		FVector position;
		if (Raycast(position)) 
		{
			streetMeshPreview->SetWorldTransform(FTransform(position));
			if (verticesPreview.Num() > 0) return; //avoid redrawing

			verticesPreview.Add(FVector::LeftVector * 500 + FVector::BackwardVector * 500 + FVector::UpVector * 10);
			verticesPreview.Add(FVector::RightVector * 500 + FVector::BackwardVector * 500 + FVector::UpVector * 10);
			verticesPreview.Add(FVector::LeftVector * 500 + FVector::ForwardVector * 500 + FVector::UpVector * 10);
			verticesPreview.Add(FVector::RightVector * 500 + FVector::ForwardVector * 500 + FVector::UpVector * 10);

			uvsPreview.Add(FVector2D(0, 0));
			uvsPreview.Add(FVector2D(0, 1));
			uvsPreview.Add(FVector2D(1, 0));
			uvsPreview.Add(FVector2D(1, 1));

			//Triangle1
			trianglesPreview.Add(0); //down left
			trianglesPreview.Add(1); //down right
			trianglesPreview.Add(2); // top left

			//Triangle2
			trianglesPreview.Add(2);
			trianglesPreview.Add(1);
			trianglesPreview.Add(3); //top right
			UE_LOG(LogTemp, Warning, TEXT("drawing preview"));

			streetMeshPreview->CreateMeshSection(0, verticesPreview, trianglesPreview, TArray<FVector>(), uvsPreview, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
		}
		return;
	}

	ClearPreviewStreet();

	streetMeshPreview->SetWorldTransform(FTransform(currentStreet->startNode.position));

	FRotator rot(0, 90, 0);
	FVector currentStreetDir = currentStreet->endNode.position - currentStreet->startNode.position;
	float streetLength = currentStreetDir.Length();
	currentStreetDir.Normalize();
	FVector currentStreetTangent = rot.RotateVector(currentStreetDir);

	FVector currentStreetVertDownLeft = currentStreetTangent * -1 * currentStreet->width * 0.5 + FVector::UpVector * 10;
	FVector currentStreetVertDownRight = currentStreetTangent * currentStreet->width * 0.5 + FVector::UpVector * 10;
	FVector currentStreetVertTopLeft = currentStreetDir * streetLength + currentStreetTangent * -1 * currentStreet->width * 0.5 + FVector::UpVector * 10;
	FVector currentStreetVertTopRight = currentStreetDir * streetLength + currentStreetTangent * currentStreet->width * 0.5 + FVector::UpVector * 10;

	verticesPreview.Add(currentStreetVertDownLeft);
	verticesPreview.Add(currentStreetVertDownRight);
	verticesPreview.Add(currentStreetVertTopLeft);
	verticesPreview.Add(currentStreetVertTopRight);

	uvsPreview.Add(FVector2D(0, 0));
	uvsPreview.Add(FVector2D(0, 1));
	uvsPreview.Add(FVector2D(1, 0));
	uvsPreview.Add(FVector2D(1, 1));

	//Triangle1
	trianglesPreview.Add(0); //down left
	trianglesPreview.Add(1); //down right
	trianglesPreview.Add(2); // top left

	//Triangle2
	trianglesPreview.Add(2);
	trianglesPreview.Add(1);
	trianglesPreview.Add(3); //top right

	streetMeshPreview->CreateMeshSection(0, verticesPreview, trianglesPreview, TArray<FVector>(), uvsPreview, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
}

void AStreetBuilder::ClearPreviewStreet()
{
	verticesPreview.Empty();
	uvsPreview.Empty();
	trianglesPreview.Empty();
	streetMeshPreview->ClearAllMeshSections();
}

bool AStreetBuilder::Raycast(FVector& outPosition) 
{
	if (APlayerController* PC = Cast<APlayerController>(Cast<AMyCamera>(GetOwner())->GetController()))
	{
		FVector mousePos;
		FVector mouseDir;
		PC->DeprojectMousePositionToWorld(mousePos, mouseDir);
		FHitResult hitResult;
		if (GetWorld()->LineTraceSingleByChannel(hitResult, mousePos, mousePos + mouseDir * 100000, ECollisionChannel::ECC_Visibility))
		{
			outPosition = hitResult.ImpactPoint;
			return true;
		}
	}

	return false;
}
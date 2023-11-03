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

}

void AStreetBuilder::PlaceRoad(const FVector& position)
{
	if (currentStreet == nullptr)
	{
		StreetNode* startNode = new StreetNode();
		startNode->position = position;
		startNode->type = StreetType::OneLane;

		StreetNode* endNode = new StreetNode();
		endNode->position = position;
		endNode->type = StreetType::OneLane;

		currentStreet = new Street();
		currentStreet->startNode = startNode;
		currentStreet->endNode = endNode;
	}
	else
	{
		previousStreet = currentStreet;
		FinishRoad();

		currentStreet = new Street();

		currentStreet->startNode = previousStreet->endNode;

		currentStreet->endNode = new StreetNode();
		currentStreet->endNode->position = position;
	}
}

void AStreetBuilder::PlacingEndRoad() 
{
	if (APlayerController* PC = Cast<APlayerController>(Cast<AMyCamera>(GetOwner())->GetController()))
	{
		FVector mousePos;
		FVector mouseDir;
		PC->DeprojectMousePositionToWorld(mousePos, mouseDir);
		FHitResult hitResult;

		if (GetWorld()->LineTraceSingleByChannel(hitResult, mousePos, mousePos + mouseDir * 100000, ECollisionChannel::ECC_Visibility))
		{
			currentStreet->endNode->position = hitResult.ImpactPoint;
		}
	}

	DrawDebugLine(GetWorld(), currentStreet->startNode->position, currentStreet->endNode->position, FColor::Blue);
	DrawDebugSphere(GetWorld(), currentStreet->startNode->position, 100, 12, FColor::Blue);
	DrawDebugSphere(GetWorld(), currentStreet->endNode->position, 100, 12, FColor::Blue);
}

void AStreetBuilder::FinishRoad() 
{
	streets.Add(currentStreet);
	currentStreet = nullptr;

	CalculateMesh();
}

void AStreetBuilder::CancelRoad() 
{
	if (previousStreet == nullptr) 
	{
		delete currentStreet->startNode;
	}
	delete currentStreet->endNode;
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

	if (streets.Num() == 1) 
	{
		CalculateFacesForStreet(streets[0], 0);

		vertices.Append(streets[0]->vertices);
		uvs.Append(streets[0]->uvs);
		triangles.Append(streets[0]->triangles);
	}
	else
	{
		Street* topStreet = streets[streets.Num() - 1];
		Street* bottomStreet = streets[streets.Num() - 2];

		bool bottomStreetCalculated = bottomStreet->vertices.Num() != 0;
		if (!bottomStreetCalculated)
			CalculateFacesForStreet(bottomStreet, streets.Num() - 2);

		bool topStreetCalculated = topStreet->vertices.Num() != 0;
		if(!topStreetCalculated)
			CalculateFacesForStreet(topStreet, streets.Num() - 1);

		SeamCorrection(bottomStreet, topStreet);

		for (const Street* street : streets) 
		{
			vertices.Append(street->vertices);
			uvs.Append(street->uvs);
			triangles.Append(street->triangles);
		}
	}

	streetMesh->CreateMeshSection(0, vertices, triangles, TArray<FVector>(), uvs, TArray<FColor>(), TArray<FProcMeshTangent>(), true);
}

void AStreetBuilder::SetMaterial(UMaterialInterface* roadMaterial)
{
	this->material = roadMaterial;
	streetMesh->SetMaterial(0, material);
}

void AStreetBuilder::CalculateFacesForStreet(Street* street, int index) 
{
	FRotator rot(0, 90, 0);
	FVector currentStreetDir = street->endNode->position - street->startNode->position;
	currentStreetDir.Normalize();
	FVector currentStreetTangent = rot.RotateVector(currentStreetDir);

	FVector currentStreetVertDownLeft = street->startNode->position + currentStreetTangent * -1 * street->width * 0.5 + FVector::UpVector * 10;
	FVector currentStreetVertDownRight = street->startNode->position + currentStreetTangent * street->width * 0.5 + FVector::UpVector * 10;
	FVector currentStreetVertTopLeft = street->endNode->position + currentStreetTangent * -1 * street->width * 0.5 + FVector::UpVector * 10;
	FVector currentStreetVertTopRight = street->endNode->position + currentStreetTangent * street->width * 0.5 + FVector::UpVector * 10;

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
	FVector bottomStreetDir = bottomStreet->endNode->position - bottomStreet->startNode->position;
	bottomStreetDir.Normalize();

	FVector topStreetVertDownLeft = topStreet->vertices[0];
	FVector topStreetVertDownRight = topStreet->vertices[1];
	FVector topStreetVertTopLeft = topStreet->vertices[2];
	FVector topStreetVertTopRight = topStreet->vertices[3];
	FVector topStreetDir = (topStreet->endNode->position - topStreet->startNode->position) * -1;
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
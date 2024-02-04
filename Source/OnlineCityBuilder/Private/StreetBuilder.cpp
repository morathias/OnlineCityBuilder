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

	if(currentStreet != nullptr && intersectingStreet == nullptr)
		PlacingEndRoad();

	AMyCityBuilderGameMode* gameMode = Cast<AMyCityBuilderGameMode>(GetWorld()->GetAuthGameMode());
	if (gameMode != nullptr && gameMode->isEditingRoad) 
	{
		DrawPreviewStreet(currentStreet != nullptr);
		FVector intersection;
		if (GetIntersectingNode(intersection))
		{
			if (currentStreet != nullptr) 
			{
				currentStreet->endNode->position = intersection;
			}
			else 
			{
				streetMeshPreview->SetWorldTransform(FTransform(intersection));
			}
		}
	}
	else
	{
		ClearPreviewStreet();
	}

	/*for (const Street* street : streets)
	{
		DrawDebugSphere(GetWorld(), street->startNode->position, street->startNode->radius, 16, FColor::Red);
		DrawDebugSphere(GetWorld(), street->endNode->position, street->endNode->radius, 16, FColor::Red);
	}*/
}

void AStreetBuilder::PlaceRoad(const FVector& position)
{
	if (currentStreet == nullptr)
	{
		//creating street from an existing one, so connect the new one to the existing one
		if (intersectingNode != nullptr)
		{
			currentStreet = new Street(intersectingNode, nullptr); //create new end node and share start node
			intersectingStreet->ConnectStreet(currentStreet);
		}
		else
		{
			currentStreet = new Street();

			currentStreet->startNode->position = position;
			currentStreet->startNode->type = StreetType::OneLane;
		}

		currentStreet->endNode->position = position;
		currentStreet->endNode->type = StreetType::OneLane;

		//connect existing road to new one
		if (intersectingStreet != nullptr) 
		{
			currentStreet->ConnectStreet(intersectingStreet);
			intersectingStreet->ConnectStreet(currentStreet);
		}
	}
	else
	{
		if (intersectingNode != nullptr)
		{
			currentStreet->UpdateEndNode(intersectingNode);

			currentStreet->ConnectStreet(intersectingStreet);
			intersectingStreet->ConnectStreet(currentStreet);

			intersectingNode = nullptr;
		}
		previousStreet = currentStreet;
		FinishRoad();

		currentStreet = new Street(previousStreet->endNode, nullptr);
		currentStreet->endNode->position = position;

		currentStreet->ConnectStreet(previousStreet);
		previousStreet->ConnectStreet(currentStreet);

		if (intersectingStreet != nullptr)
		{
			currentStreet->ConnectStreet(intersectingStreet);
			intersectingStreet->ConnectStreet(currentStreet);
		}
	}

}

void AStreetBuilder::PlacingEndRoad() 
{
	FVector position;
	if (Raycast(position))
	{
		currentStreet->endNode->position = position;
	}

	/*if (currentStreet != nullptr)
	{
		DrawDebugLine(GetWorld(), currentStreet->startNode->position, currentStreet->endNode->position, FColor::Blue);
		DrawDebugSphere(GetWorld(), currentStreet->startNode->position, 100, 12, FColor::Blue);
		DrawDebugSphere(GetWorld(), currentStreet->endNode->position, 100, 12, FColor::Blue);
	}*/
}

void AStreetBuilder::FinishRoad() 
{
	streets.Add(currentStreet);

	MarkNodeAsDirty(currentStreet->startNode);
	MarkNodeAsDirty(currentStreet->endNode);

	currentStreet = nullptr;

	CalculateMesh();
}

void AStreetBuilder::CancelRoad() 
{
	if (currentStreet == nullptr) return; //no street to cancel

	delete currentStreet;
	currentStreet = nullptr;

	previousStreet = nullptr;

	ClearPreviewStreet();
}

void AStreetBuilder::CalculateMesh() 
{
	triangles.Empty();
	vertices.Empty();
	uvs.Empty();
	streetMesh->ClearAllMeshSections();

	Street* lastPlacedStreet = streets[streets.Num() - 1];
	CalculateFacesForStreet(lastPlacedStreet, streets.Num() - 1);
	SeamCorrection(lastPlacedStreet);

	dirtyNodes.Empty();

	for (const Street* street : streets) 
	{
		vertices.Append(street->vertices);
		uvs.Append(street->uvs);
		triangles.Append(street->triangles);
		if(street->startNode->holeIndices.Num() > 0)
			triangles.Append(street->startNode->holeIndices);
		if (street->endNode->holeIndices.Num() > 0)
			triangles.Append(street->endNode->holeIndices);
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

void AStreetBuilder::SeamCorrection(Street* topStreet) 
{
	//for each node vertex, check intersection with all connected streets borders
	//Get nearest intersection point to vertex
	//if two intersection point are at same distance to vertex, get the point existing in the current street border
	//if an intersection point is at the same distance as the vertex, discard intersection point

	if (dirtyNodes.Contains(topStreet->endNode))//only nodes that were modified need to be calculated
	{ 
		GetIntersectionForNodeVertices(*topStreet->endNode);
		CalculateHolesIndeces(*topStreet->endNode);
	}
	if (dirtyNodes.Contains(topStreet->startNode))
	{
		GetIntersectionForNodeVertices(*topStreet->startNode);
		CalculateHolesIndeces(*topStreet->startNode);
	}
}

void AStreetBuilder::GetIntersectionForNodeVertices(const Street::Node& node)
{
	if (node.owners.Num() == 1) return; //no correction needed
	TArray<Street*> sortedStreets = SortStreetsClockwise(node);

	for (int i = 0; i < node.owners.Num(); i++)
	{
		node.owners[i]->ResetVerticesPositions(&node);
	}

	for (int i = 1; i < sortedStreets.Num(); i++)
	{
		TArray<FVector*> streetVertices = sortedStreets[i - 1]->GetVerticesForNode(&node);//index 0 is always left vertex in relation of start node
		TArray<FVector*> otherStreetVertices = sortedStreets[i]->GetVerticesForNode(&node);

		FVector& leftVert = &node == sortedStreets[i - 1]->endNode ? *streetVertices[1] : *streetVertices[0]; // get the left vertex in relation to center of streets
		FVector& rightVert = &node == sortedStreets[i]->endNode ? *otherStreetVertices[0] : *otherStreetVertices[1]; // get the right vertex in relation to center of streets

		FVector leftVertexDir = &node == sortedStreets[i - 1]->endNode ? (sortedStreets[i - 1]->startNode->position - sortedStreets[i - 1]->endNode->position) :
										(sortedStreets[i - 1]->endNode->position - sortedStreets[i - 1]->startNode->position);
		leftVertexDir.Normalize();

		FVector rightVertexDir = &node == sortedStreets[i]->endNode ? (sortedStreets[i]->startNode->position - sortedStreets[i]->endNode->position) :
										 (sortedStreets[i]->endNode->position - sortedStreets[i]->startNode->position);
		rightVertexDir.Normalize();

		IntersectVertices(rightVert, rightVertexDir, leftVert, leftVertexDir);
	}

	//do the same but for first and last streets
	TArray<FVector*> streetVertices = sortedStreets[sortedStreets.Num() - 1]->GetVerticesForNode(&node);
	TArray<FVector*> otherStreetVertices = sortedStreets[0]->GetVerticesForNode(&node);

	FVector& leftVert = &node == sortedStreets[sortedStreets.Num() - 1]->endNode ? *streetVertices[1] : *streetVertices[0];
	FVector& rightVert = &node == sortedStreets[0]->endNode ? *otherStreetVertices[0] : *otherStreetVertices[1];

	FVector leftVertexDir = &node == sortedStreets[sortedStreets.Num() - 1]->endNode ? (sortedStreets[sortedStreets.Num() - 1]->startNode->position - sortedStreets[sortedStreets.Num() - 1]->endNode->position) :
									(sortedStreets[sortedStreets.Num() - 1]->endNode->position - sortedStreets[sortedStreets.Num() - 1]->startNode->position);
	leftVertexDir.Normalize();

	FVector rightVertexDir = &node == sortedStreets[0]->endNode ? (sortedStreets[0]->startNode->position - sortedStreets[0]->endNode->position) :
									  (sortedStreets[0]->endNode->position - sortedStreets[0]->startNode->position);
	rightVertexDir.Normalize();
	
	IntersectVertices(rightVert, rightVertexDir, leftVert, leftVertexDir);
	
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

			streetMeshPreview->CreateMeshSection(0, verticesPreview, trianglesPreview, TArray<FVector>(), uvsPreview, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
		}
		return;
	}

	if (currentStreet == nullptr) return;

	ClearPreviewStreet();

	streetMeshPreview->SetWorldTransform(FTransform(currentStreet->startNode->position));

	FRotator rot(0, 90, 0);
	FVector currentStreetDir = currentStreet->endNode->position - currentStreet->startNode->position;
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

bool AStreetBuilder::GetIntersectingNode(FVector& outIntersection)
{
	for (Street* street : streets)
	{
		FVector position;
		Raycast(position);
		FVector closestPoint = FMath::ClosestPointOnLine(street->startNode->position, street->endNode->position, position);
		float distanceToStreet = (closestPoint - position).Length();

		//DrawDebugLine(GetWorld(), position, closestPoint, FColor::Green);

		//Get intersecting Street
		if (distanceToStreet <= street->width * 0.5) 
		{
			//DrawDebugSphere(GetWorld(), closestPoint, street->startNode->radius, 12, FColor::Green);
			outIntersection = closestPoint;
			intersectingStreet = street;

			//Get intersecting node
			if ((position - street->startNode->position).Length() <= street->startNode->radius)
			{
				outIntersection = street->startNode->position;
				intersectingNode = street->startNode;
				return true;
			}
			else
			{
				intersectingNode = nullptr;
			}

			if ((position - street->endNode->position).Length() <= street->endNode->radius)
			{
				outIntersection = street->endNode->position;
				intersectingNode = street->endNode;
				return true;
			}
			else
			{
				intersectingNode = nullptr;
			}

			return true; //return in case no intersection node was found
		}
	}

	intersectingStreet = nullptr;
	intersectingNode = nullptr;
	return false;
}

void AStreetBuilder::MarkNodeAsDirty(Street::Node* dirtyNode) 
{
	if (!dirtyNodes.Contains(dirtyNode))
		dirtyNodes.Add(dirtyNode);
}

TArray<Street*> AStreetBuilder::SortStreetsClockwise(const Street::Node& referenceNode) 
{
	TArray<Street*> sortedStreets = referenceNode.owners;
	sortedStreets.Sort([&](Street& streetA, Street& streetB)
		{
			FVector center = referenceNode.position;
			FVector a = FMath::Lerp(streetA.endNode->position, streetA.startNode->position, 0.5);
			FVector b = FMath::Lerp(streetB.endNode->position, streetB.startNode->position, 0.5);

			if (a.X - center.X >= 0 && b.X - center.X < 0)
				return true;
			if (a.X - center.X < 0 && b.X - center.X >= 0)
				return false;
			if (a.X - center.X == 0 && b.X - center.X == 0) {
				if (a.Y - center.Y >= 0 || b.Y - center.Y >= 0)
					return a.Y > b.Y;
				return b.Y > a.Y;
			}

			// compute the cross product of vectors (center -> a) x (center -> b)
			int det = (a.X - center.X) * (b.Y - center.Y) - (b.X - center.X) * (a.Y - center.Y);
			if (det < 0)
				return true;
			if (det > 0)
				return false;

			// points a and b are on the same line from the center
			// check which point is closer to the center
			int d1 = (a.X - center.X) * (a.X - center.X) + (a.Y - center.Y) * (a.Y - center.Y);
			int d2 = (b.X - center.X) * (b.X - center.X) + (b.Y - center.Y) * (b.Y - center.Y);
			return d1 > d2;
		});

	return sortedStreets;
}

void AStreetBuilder::IntersectVertices(FVector& rightVert, FVector rightVertDir, FVector& leftVert, FVector leftVertDir) 
{
	FVector leftvertexStartPos = leftVert + (leftVertDir * -1) * 100000;
	FVector leftvertexEndPos = leftVert + leftVertDir * 100000;

	FVector rightVertexStartPos = rightVert + (rightVertDir * -1) * 100000;
	FVector rightVertexEndPos = rightVert + rightVertDir * 100000;

	FVector intersection = FVector::Zero();
	FMath::SegmentIntersection2D(leftvertexStartPos, leftvertexEndPos, rightVertexStartPos, rightVertexEndPos, intersection);

	leftVert = intersection;
	rightVert = intersection;
}

void AStreetBuilder::CalculateHolesIndeces(Street::Node& nodeToFill) 
{
	if (nodeToFill.owners.Num() <= 2) return;

	nodeToFill.holeIndices.Empty();

	TArray<FVector> nodeVertices;
	TArray<Street*> sortedStreets = SortStreetsClockwise(nodeToFill);

	Street* firstStreet = sortedStreets[0];
	int firstStreetIndex = streets.IndexOfByKey(firstStreet);
	TArray<int> firstStreetIndeces = firstStreet->GetIndecesForNode(&nodeToFill);
	int pivotVertexIndex = &nodeToFill == firstStreet->endNode ? (firstStreetIndex * 4 + firstStreetIndeces[0]) : (firstStreetIndex * 4 + firstStreetIndeces[1]);

	//we don't need the last street to fill the hole
	for (int i = 1; i < sortedStreets.Num() - 1; i++)
	{
		int streetIndex = streets.IndexOfByKey(sortedStreets[i]);

		TArray<int> otherStreetIndeces = sortedStreets[i]->GetIndecesForNode(&nodeToFill);

		int leftVertexIndex = &nodeToFill == sortedStreets[i]->endNode ? (streetIndex * 4 + otherStreetIndeces[0]) : (streetIndex * 4 + otherStreetIndeces[1]);
		int rightVertexIndex = &nodeToFill == sortedStreets[i]->endNode ? (streetIndex * 4 + otherStreetIndeces[1]) : (streetIndex * 4 + otherStreetIndeces[0]);

		nodeToFill.holeIndices.Add(leftVertexIndex);
		nodeToFill.holeIndices.Add(rightVertexIndex);
		nodeToFill.holeIndices.Add(pivotVertexIndex);
	}
}

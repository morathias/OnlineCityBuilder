// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Street.h"
#include "StreetBuilder.generated.h"

class UProceduralMeshComponent;

UCLASS()
class ONLINECITYBUILDER_API AStreetBuilder : public AActor
{
	GENERATED_BODY()
	
public:	

	// Sets default values for this actor's properties
	AStreetBuilder();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void PlaceRoad(const FVector& position);
	void FinishRoad();
	void CancelRoad();

	void SetMaterial(UMaterialInterface* material);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	

	enum Vertices
	{
		BottomLeft,
		BottomRight,
		TopLeft,
		TopRight,
	};

	void PlacingEndRoad();
	void CalculateMesh();
	void CalculateFacesForStreet(Street* street, int index = 0);
	void SeamCorrection(Street* topStreet);

	void DrawPreviewStreet(bool isPlacing);
	void ClearPreviewStreet();

	bool Raycast(FVector& outPosition);

	bool GetIntersectingNode(FVector& outIntersection);
	void GetIntersectionForNodeVertices(const Street::Node& node);
	void IntersectVertices(FVector& rightVert, FVector rightVertDir, FVector& leftVert, FVector leftVertDir);
	void SplitStreet(Street* streetToSplit, Street::Node* splitNode);
	void MarkNodeAsDirty(Street::Node* dirtyNode);
	TArray<Street*> SortStreetsClockwise(const Street::Node& referenceNode);
	void CalculateHolesIndeces(Street::Node& nodeToFill);

	TArray<Street*> streets;
	Street* currentStreet;
	Street* previousStreet;
	Street* intersectingStreet;
	Street::Node* intersectingNode;
	TArray<Street::Node*> dirtyNodes;

	UPROPERTY()
	UProceduralMeshComponent* streetMesh;
	TArray<FVector> vertices;
	TArray<int32> triangles;
	TArray<FVector2D> uvs;
	UMaterialInterface* material;

	UProceduralMeshComponent* streetMeshPreview;
	TArray<FVector> verticesPreview;
	TArray<int32> trianglesPreview;
	TArray<FVector2D> uvsPreview;
	UMaterialInterface* materialPreview;
};

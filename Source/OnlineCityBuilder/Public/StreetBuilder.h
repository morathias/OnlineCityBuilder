// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StreetBuilder.generated.h"

enum StreetType;
class UProceduralMeshComponent;

UCLASS()
class ONLINECITYBUILDER_API AStreetBuilder : public AActor
{
	GENERATED_BODY()
	
public:	

	struct StreetNode
	{
	public: 
		FVector position;
		float radius = 500;
		StreetType type;
	};

	struct Street 
	{
	public: 
		StreetNode* startNode;
		StreetNode* endNode;

		float width = 1000;

		TArray<FVector> vertices;
		TArray<int32> triangles;
		TArray<FVector2D> uvs;

		TArray<const Street*> connectedStreets;
	};
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
	void PlacingEndRoad();
	void CalculateMesh();
	void CalculateFacesForStreet(Street* street, int index = 0);
	void SeamCorrection(Street* bottomStreet, Street* topStreet);

	void DrawPreviewStreet(bool isPlacing);
	void ClearPreviewStreet();

	bool Raycast(FVector& outPosition);

	TArray<Street*> streets;
	Street* currentStreet;
	Street* previousStreet;

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

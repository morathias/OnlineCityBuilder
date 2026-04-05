// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildingViewData.h"
#include "ABuilding.generated.h"

class UProceduralMeshComponent;

UCLASS()
class ONLINECITYBUILDER_API ABuilding : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABuilding();
	virtual void Tick(float DeltaTime) override;

	void StartBuilding(TArray<FVector> area);

	FIntVector2 storeysRange;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	
	void Building(float dt);
	void CalculateMesh(TArray<FVector> area, FVector dimensions);
	void BuildMesh(const UStaticMesh& mesh, TArray<FVector> buildArea, uint8 inStoreys, TArray<FVector>& currentBuildingVerts, TArray<int32>& currentBuildingIndices, TArray<FVector>& currentBuildingNormals, int amount = -1, bool removeCorners = false);
	void BuildMeshGrid(const UStaticMesh& mesh, TArray<FVector> buildArea, TArray<FVector>& currentBuildingVerts, TArray<int32>& currentBuildingIndices, TArray<FVector>& currentBuildingNormals);

	const float MAX_STORY_HEIGHT = 3.65 * 100;
	int32 storeys;

	//width, depth
	FVector2D minDimensions, maxDimensions;

	float buildingProgress;

	DECLARE_DELEGATE(OnBuildingConstructed);
	OnBuildingConstructed onBuildingConstructed;

	TArray<FVector> vertices;
	UProceduralMeshComponent* buildingMesh;

	UPROPERTY(EditAnywhere) UBuildingViewData* viewData;
};

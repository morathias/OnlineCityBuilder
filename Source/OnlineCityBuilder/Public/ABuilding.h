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
	float AngleBetweenVectors(FVector Vec1, FVector Vec2);


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

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Zone.h"
#include "ZoneBuilder.generated.h"

class UProceduralMeshComponent;

UCLASS()
class ONLINECITYBUILDER_API AZoneBuilder : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AZoneBuilder();

	virtual void Tick(float DeltaTime) override;

	void SetMaterial(UMaterialInterface* mat);
	void GenerateZone(TArray<FVector> vertices, TArray<FVector> normals);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void CalculateMesh();

	UMaterialInterface* zoneMaterial;
	TArray<Zone*> zones;

	UProceduralMeshComponent* mesh;
	TArray<FVector> meshVertices;
	TArray<int> meshIndices;
};

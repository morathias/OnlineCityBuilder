// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyCamera.generated.h"


class AStreetBuilder;

UCLASS()
class ONLINECITYBUILDER_API AMyCamera : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMyCamera();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Move(FVector2D value);
	void Zoom(float value);

	void PlaceObject();
	void CancelPlacement();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	

	UPROPERTY(EditAnywhere, Category = "Road Editing")
	float rayLength = 10000;

	UPROPERTY(EditAnywhere, Category = "Road Editing")
	UMaterialInterface* roadMaterial;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float speed = 10;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float zoomSpeed = 10;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float smoothing = 5;

	FVector movementDir;
	FVector targetPos;

	AStreetBuilder* streetBuilder;
};

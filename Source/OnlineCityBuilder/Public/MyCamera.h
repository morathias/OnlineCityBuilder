// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InputActionValue.h"

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyCamera.generated.h"

class UInputMappingContext;
class UInputAction;
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

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* inputMapping;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* moveInputAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* zoomInputAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* placeObjectInputAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* cancelPlacementInputAction;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float speed = 10;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float zoomSpeed = 10;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float smoothing = 5;

	UPROPERTY(EditAnywhere, Category = "Road Editing")
	float rayLength = 10000;

	UPROPERTY(EditAnywhere, Category = "Road Editing")
	UMaterialInterface* roadMaterial;

	FVector movementDir;
	FVector targetPos;

	AStreetBuilder* streetBuilder;

	void Move(const FInputActionValue& value);
	void Zoom(const FInputActionValue& value);

	void PlaceObject(const FInputActionValue& value);
	void CancelPlacement(const FInputActionValue& value);
};

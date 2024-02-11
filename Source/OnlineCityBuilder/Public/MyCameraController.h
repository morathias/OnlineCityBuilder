// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InputActionValue.h"

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyCameraController.generated.h"

class UInputMappingContext;
class UInputAction;
class AMyCamera;

UCLASS()
class ONLINECITYBUILDER_API AMyCameraController : public APlayerController
{
	GENERATED_BODY()

public:
	AMyCameraController();
	// Called to bind functionality to input
	virtual void SetupInputComponent() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	void Move(const FInputActionValue& value);
	void Zoom(const FInputActionValue& value);
	void Rotate(const FInputActionValue& value);

	void PlaceObject(const FInputActionValue& value);
	void CancelPlacement(const FInputActionValue& value);


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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* rotateInputAction;

	AMyCamera* possessedPawn;
};

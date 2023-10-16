// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InputActionValue.h"

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyCamera.generated.h"

class UInputAction;

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
	class UInputMappingContext* inputMapping;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* moveInputAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* zoomInputAction;
	
	UPROPERTY(EditAnywhere)
	float speed = 10;

	UPROPERTY(EditAnywhere)
	float zoomSpeed = 10;

	UPROPERTY(EditAnywhere)
	float smoothing = 5;

	FVector movementDir;
	FVector targetPos;

	void Move(const FInputActionValue& value);
	void Zoom(const FInputActionValue& value);
};

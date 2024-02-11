// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCameraController.h"
#include "MyCamera.h"

#include <InputMappingContext.h>
#include <InputAction.h>
#include <EnhancedInputSubsystems.h>
#include <EnhancedInputComponent.h>

AMyCameraController::AMyCameraController() {}

void AMyCameraController::BeginPlay() 
{
    possessedPawn = GetPawn<AMyCamera>();
    Possess(possessedPawn);
}

// Called to bind functionality to input
void AMyCameraController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Get the local player subsystem
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        // Clear out existing mapping, and add our mapping
        Subsystem->ClearAllMappings();
        Subsystem->AddMappingContext(inputMapping, 0);
    }

    if (UEnhancedInputComponent* playerEnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (moveInputAction)
        {
            playerEnhancedInputComponent->BindAction(moveInputAction, ETriggerEvent::Triggered, this, &AMyCameraController::Move);
            playerEnhancedInputComponent->BindAction(moveInputAction, ETriggerEvent::Completed, this, &AMyCameraController::Move);
        }

        if (zoomInputAction)
        {
            playerEnhancedInputComponent->BindAction(zoomInputAction, ETriggerEvent::Triggered, this, &AMyCameraController::Zoom);
            playerEnhancedInputComponent->BindAction(zoomInputAction, ETriggerEvent::Completed, this, &AMyCameraController::Zoom);
        }

        if (placeObjectInputAction)
        {
            playerEnhancedInputComponent->BindAction(placeObjectInputAction, ETriggerEvent::Started, this, &AMyCameraController::PlaceObject);
        }

        if (cancelPlacementInputAction)
        {
            playerEnhancedInputComponent->BindAction(cancelPlacementInputAction, ETriggerEvent::Started, this, &AMyCameraController::CancelPlacement);
        }
    }
}

void AMyCameraController::Move(const FInputActionValue& value)
{
    possessedPawn->Move(value.Get<FVector2D>());
}

void AMyCameraController::Zoom(const FInputActionValue& value)
{
    possessedPawn->Zoom(value.Get<float>());
}

void AMyCameraController::PlaceObject(const FInputActionValue& value)
{
    if (value.Get<bool>())
        possessedPawn->PlaceObject();
}

void AMyCameraController::CancelPlacement(const FInputActionValue& value)
{
    if (value.Get<bool>())
        possessedPawn->CancelPlacement();
}
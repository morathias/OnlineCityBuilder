// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCamera.h"

#include <InputMappingContext.h>
#include <InputAction.h>
#include <EnhancedInputSubsystems.h>
#include <EnhancedInputComponent.h>

// Sets default values
AMyCamera::AMyCamera()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    movementDir = FVector::ZeroVector;
    targetPos = GetActorLocation();
}

// Called when the game starts or when spawned
void AMyCamera::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMyCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    SetActorLocation(FMath::Lerp(GetActorLocation(), targetPos, smoothing * DeltaTime));
}

void AMyCamera::Move(const FInputActionValue& value)
{
    FVector2D axis = value.Get<FVector2D>();
    movementDir.X = axis.X;
    movementDir.Y = axis.Y;

    FVector forward = GetActorForwardVector();
    FVector right = GetActorRightVector();

    FVector translation = (forward * movementDir.Y + right * movementDir.X);
    translation.Normalize();
    translation *= speed;

    targetPos += translation;
}

void AMyCamera::Zoom(const FInputActionValue& value)
{
    movementDir.Z = value.Get<float>();

    FVector up = GetActorUpVector();
    FVector zoom = up * movementDir.Z * zoomSpeed;

    targetPos += zoom;
}

// Called to bind functionality to input
void AMyCamera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        // Get the local player subsystem
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            // Clear out existing mapping, and add our mapping
            Subsystem->ClearAllMappings();
            Subsystem->AddMappingContext(inputMapping, 0);
        }
    }

    if (UEnhancedInputComponent* playerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (moveInputAction)
        {
            playerEnhancedInputComponent->BindAction(moveInputAction, ETriggerEvent::Triggered, this, &AMyCamera::Move);
            playerEnhancedInputComponent->BindAction(moveInputAction, ETriggerEvent::Completed, this, &AMyCamera::Move);
        }

        if (zoomInputAction) 
        {
            playerEnhancedInputComponent->BindAction(zoomInputAction, ETriggerEvent::Triggered, this, &AMyCamera::Zoom);
            playerEnhancedInputComponent->BindAction(zoomInputAction, ETriggerEvent::Completed, this, &AMyCamera::Zoom);
        }
    }
}


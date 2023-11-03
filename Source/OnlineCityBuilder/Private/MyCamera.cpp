// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCamera.h"

#include "StreetBuilder.h"

#include <InputMappingContext.h>
#include <InputAction.h>
#include <EnhancedInputSubsystems.h>
#include <EnhancedInputComponent.h>
#include "MyCityBuilderGameMode.h"

// Sets default values
AMyCamera::AMyCamera()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMyCamera::BeginPlay()
{
	Super::BeginPlay();

    movementDir = FVector::ZeroVector;
    targetPos = GetActorLocation();

    streetBuilder = GetWorld()->SpawnActor<AStreetBuilder>(AStreetBuilder::StaticClass());
    streetBuilder->SetOwner(this);
    streetBuilder->SetMaterial(roadMaterial);
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

void AMyCamera::PlaceObject(const FInputActionValue& value)
{
    if (!value.Get<bool>()) return;

    AMyCityBuilderGameMode* gameMode = Cast<AMyCityBuilderGameMode>(GetWorld()->GetAuthGameMode());
    if (gameMode == nullptr || !gameMode->isEditingRoad) return;

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        FVector mousePos;
        FVector mouseDir;
        PC->DeprojectMousePositionToWorld(mousePos, mouseDir);
        FHitResult hitResult;

        if (GetWorld()->LineTraceSingleByChannel(hitResult, mousePos, mousePos + mouseDir * rayLength, ECollisionChannel::ECC_Visibility))
        {
            streetBuilder->PlaceRoad(hitResult.ImpactPoint);
        }
    }
}

void AMyCamera::CancelPlacement(const FInputActionValue& value) 
{
    if (!value.Get<bool>()) return;

    AMyCityBuilderGameMode* gameMode = Cast<AMyCityBuilderGameMode>(GetWorld()->GetAuthGameMode());
    if (gameMode == nullptr || !gameMode->isEditingRoad) return;

    streetBuilder->CancelRoad();
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

        if (placeObjectInputAction)
        {
            playerEnhancedInputComponent->BindAction(placeObjectInputAction, ETriggerEvent::Started, this, &AMyCamera::PlaceObject);
        }

        if (cancelPlacementInputAction)
        {
            playerEnhancedInputComponent->BindAction(cancelPlacementInputAction, ETriggerEvent::Started, this, &AMyCamera::CancelPlacement);
        }
    }
}


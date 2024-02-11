// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCamera.h"

#include "StreetBuilder.h"
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

void AMyCamera::Move(FVector2D value)
{
    FVector2D axis = value;
    movementDir.X = axis.X;
    movementDir.Y = axis.Y;

    FVector forward = GetActorForwardVector();
    FVector right = GetActorRightVector();

    FVector translation = (forward * movementDir.Y + right * movementDir.X);
    translation.Normalize();
    translation *= speed;

    targetPos += translation;
}

void AMyCamera::Zoom(float value)
{
    movementDir.Z = value;

    FVector up = GetActorUpVector();
    FVector zoom = up * movementDir.Z * zoomSpeed;

    targetPos += zoom;
}

void AMyCamera::PlaceObject()
{
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

void AMyCamera::CancelPlacement() 
{
    AMyCityBuilderGameMode* gameMode = Cast<AMyCityBuilderGameMode>(GetWorld()->GetAuthGameMode());
    if (gameMode == nullptr || !gameMode->isEditingRoad) return;

    streetBuilder->CancelRoad();
}




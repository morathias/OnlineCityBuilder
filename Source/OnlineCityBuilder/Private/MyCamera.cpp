// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCamera.h"

#include "StreetBuilder.h"
#include "MyCityBuilderGameMode.h"
#include "ZoneBuilder.h"

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
    streetBuilder->streetPlacedDelegate.BindUObject(this, &AMyCamera::GetStreetBordersForZone);

    zoneBuilder = GetWorld()->SpawnActor<AZoneBuilder>(AZoneBuilder::StaticClass());
    zoneBuilder->SetOwner(this);
    zoneBuilder->SetMaterial(zoneMaterial);


    SetActorRotation(FRotator(-45, 0, 0));
}

// Called every frame
void AMyCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    CalculateCurrentSpeed();
    SetActorLocation(FMath::Lerp(GetActorLocation(), targetPos, smoothing * DeltaTime));
}

void AMyCamera::Move(FVector2D value)
{
    FVector2D axis = value;
    movementDir.X = axis.X;
    movementDir.Y = axis.Y;

    FVector forward = GetActorForwardVector();
    forward.Z = 0;
    forward.Normalize();
    FVector right = GetActorRightVector();

    FVector translation = (forward * movementDir.Y + right * movementDir.X);
    translation.Normalize();
    translation *= currentSpeed;

    targetPos += translation;
}

void AMyCamera::Zoom(float value)
{
    movementDir.Z = value;

    FVector up = FVector::UpVector;
    FVector zoom = up * movementDir.Z * zoomSpeed;

    targetPos += zoom;

    targetPos.Z = FMath::Clamp(targetPos.Z, verticalLimit.X, verticalLimit.Y);
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

void AMyCamera::CalculateCurrentSpeed() 
{
    currentSpeed = FMath::Lerp(speedRange.X, speedRange.Y, GetActorLocation().Z / verticalLimit.Y);
}

void AMyCamera::Rotate(FVector2D mouseDelta) 
{
    FRotator currentRot = GetActorRotation();

    float yaw = currentRot.Yaw + mouseDelta.X;
    float pitch = currentRot.Pitch + mouseDelta.Y;
    FRotator targetRot = FRotator(pitch, yaw, currentRot.Roll);

    SetActorRotation(targetRot);
}

void AMyCamera::GetStreetBordersForZone() 
{
    zoneBuilder->GenerateZone(streetBuilder->GetLeftBorder(), streetBuilder->GetLeftBorderNormals());
    zoneBuilder->GenerateZone(streetBuilder->GetRightBorder(), streetBuilder->GetRightBorderNormals());
}
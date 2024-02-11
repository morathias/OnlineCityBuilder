// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCityBuilderGameMode.h"
#include "MyCamera.h"
#include "MyCameraController.h"

AMyCityBuilderGameMode::AMyCityBuilderGameMode()
{
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;
    PrimaryActorTick.bAllowTickOnDedicatedServer = false;
}
AMyCityBuilderGameMode::~AMyCityBuilderGameMode() {}

void AMyCityBuilderGameMode::IsEditingRoad(bool isEditing) 
{
    this->isEditingRoad = isEditing;
}
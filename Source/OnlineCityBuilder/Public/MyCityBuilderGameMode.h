// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyCityBuilderGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ONLINECITYBUILDER_API AMyCityBuilderGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMyCityBuilderGameMode();
	~AMyCityBuilderGameMode();

	

	UFUNCTION(BlueprintCallable)
	void IsEditingRoad(bool isEditing);

	bool isEditingRoad = false;
};

UENUM()
enum StreetType
{
	OneLane		UMETA(DisplayName = "1 Lane"),
	TwoLane		UMETA(DisplayName = "2 Lane"),
	Avenue		UMETA(DisplayName = "Avenue"),
	Collector	UMETA(DisplayName = "Collector"),
	Highway		UMETA(DisplayName = "Highway")
};

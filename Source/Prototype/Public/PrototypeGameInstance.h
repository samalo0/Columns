// Copyright 2019
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PrototypeGameInstance.generated.h"

UCLASS()
class PROTOTYPE_API UPrototypeGameInstance : public UGameInstance
{

	GENERATED_BODY()

public:
	// The current level of the game
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Instance")
	int32 Level = 1;

	// The score of the game, only updated when changing levels
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Instance")
	int32 Score = 0;
};

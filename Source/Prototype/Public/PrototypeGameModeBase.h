// Copyright 2019
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PrototypeGameModeBase.generated.h"

UCLASS()
class PROTOTYPE_API APrototypeGameModeBase : public AGameModeBase
{
	
	GENERATED_BODY()

public:

	// Adds a number of jewels and calculates score adder.
	void AddToJewelsAndScore(const int32 jewels);

	// Returns the number of jewels for the current level.
	FORCEINLINE int32 GetJewels() const { return Jewels; }

	// Returns the number of jewels to be collected to advance level.
	FORCEINLINE int32 GetJewelsRequired() const { return JewelsRequired; }

	// Returns the current level.
	FORCEINLINE int32 GetLevel() const { return Level; }

	// Blueprint event to trigger ending the game.
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void TriggerEndGame();

	// Blueprint even to trigger advancing to the next level.
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void TriggerNextLevel();

protected:

	// The text displayed on the center of the screen (to indicate game over, next level).
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GameMode")
	FText CenteredText;

	// The number of collected jewels in the current level.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GameMode")
	int32 Jewels = 0;

	// The number of jewels required to advance level.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GameMode")
	int32 JewelsRequired = 20;

	// The height at which random blocks are added to the board.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GameMode")
	int32 Level = 1;

	// The total score.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GameMode")
	int32 Score;
};

// Copyright 2019
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameBoardActor.h"
#include "PrototypePawn.generated.h"

// Set the number of symbols in the pawn.
constexpr int32 PAWN_SIZE = 3;

// Sets the pawn speed when transitioning between locations.
constexpr int32 PAWN_SPEED_PIXELS_PER_SECOND = 1200;

// Sets the pixels per second that gravity pulls down the symbol (adjusted by level)
constexpr int32 PAWN_GRAVITY_PIXELS_PER_SECOND = 90;

// how many pixels per second to add to gravity per level
constexpr float PAWN_GRAVITY_LEVEL_SCALE = 20;

UCLASS()
class PROTOTYPE_API APrototypePawn : public APawn
{

	GENERATED_BODY()

public:

	APrototypePawn();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

	// Called by the game board after animation completes to end the trigger next move process.
	void TriggerNextMoveEnd();

protected:

	virtual void BeginPlay() override;

	// Construct the stacked symbols that represent the pawn at the location.
	void ConstructTrinity();

	// Reorder the symbols by moving them down (last symbol goes to top).
	void ShuffleDown();
	
	// Reorder the symbols by moving them up (top symbol goes to bottom).
	void ShuffleUp();

	// Attempt to move the trinity down; key first pressed.
	void MoveDownPressed();
	
	// Move down key released; stop travling down quickly.
	void MoveDownReleased();

	// Attempt to move the trinity left.
	void MoveLeft();

	// Attempt to move the trinity right.
	void MoveRight();

	// A collision downward occurred; trigger next move preparation. Defer to game board to allow collapsing animation.
	void TriggerNextMoveStart();

	// Components that make up the pawn visual.
	TArray<UStaticMeshComponent*> CurrentPawnStaticMeshComponents;

	// Symbol indicies that make up the pawn.
	TArray<int32> CurrentSymbolIndicies;

	// variables use to smoothly move between locations
	int32 DesiredLocationX = 0;
	int32 DesiredLocationY = -PAWN_SIZE;

	// The game board instance that the pawn moves across.
	UPROPERTY(EditAnywhere, Category="PrototypePawn")
	AGameBoardActor *GameBoardActor;

	// The gravity used to pull down the symbols during play. Scaled down based on level.
	float GravityPixelsPerSecond = PAWN_GRAVITY_PIXELS_PER_SECOND;

	// whether the move down key is being held to trigger moving down more quickly
	bool MoveDownKeyHeldDown = false;

	// The components that represent the next set of symbols.
	TArray<UStaticMeshComponent*> NextPawnStaticMeshComponents;

	// Symbol indicies that make up the next set of symbols.
	TArray<int32> NextSymbolIndicies;

	// Column index where the top symbol is located.
	int32 LocationX = 0;

	// Row index where the top symbol is located.
	int32 LocationY = -PAWN_SIZE;

	// Root scene component so that this pawn is visible in level.
	USceneComponent* RootSceneComponent;

	// Array of static mesh objects to use as symbols
	UPROPERTY(EditAnywhere, Category = "PrototypePawn")
	TArray<UStaticMesh*> SymbolStaticMeshArray;
};

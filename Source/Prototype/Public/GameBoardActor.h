// Copyright 2019
#pragma once

#include "CoreMinimal.h"
#include "FRowColumn.h"
#include "GameFramework/Actor.h"
#include "GameBoardActor.generated.h"

// Set the game board number of columns (width).
constexpr int32 GAME_BOARD_NUMBER_OF_COLUMNS = 6;

// Set the game board number of rows (length).
constexpr int32 GAME_BOARD_NUMBER_OF_ROWS = 13;

// Set the space of the grid and distance between elements.
constexpr float GAME_BOARD_SPACING = 100.0f;

// Amount of time that the highlight animation is played when removing symbols.
constexpr float SYMBOL_HIGHLIGHT_SECONDS = 0.5f;

// Used for controlling tick animation state.
UENUM(BlueprintType)
enum class EAnimationState : uint8
{
	IDLE UMETA(DisplayName = "Idle"),
	SYMBOLS UMETA(DisplayName = "Symbols"),
	EMPTY UMETA(DisplayName = "Empty"),
};

// Used to show which direction adjacent matching symbols are found.
UENUM(BlueprintType)
enum class EDirections : uint8
{
	INDETERMINATE UMETA(DisplayName = "Indeterminate"),
	LEFT UMETA(DisplayName = "Left"),
	DOWN_LEFT UMETA(DisplayName = "DownLeft"),
	DOWN UMETA(DisplayName = "Down"),
	DOWN_RIGHT UMETA(DisplayName = "DownRight"),
	RIGHT UMETA(DisplayName = "Right"),
	UP_RIGHT UMETA(DisplayName = "UpRight"),
	UP UMETA(DisplayName = "Up"),
	UP_LEFT UMETA(DisplayName = "UpLeft"),
};

// Holds the state of the game, and static mesh components that visually represent it.
UCLASS()
class PROTOTYPE_API AGameBoardActor : public AActor
{
		
	GENERATED_BODY()
	
public:	
	   
	AGameBoardActor();

	// Get the symbol located at the row and column.
	int32 BoardGet(const int32 row, const int32 column) const;

	// Set a trinity of symbols into the array and update the static mesh components.
	void BoardSetTrinity(TArray<int32> symbolsArray, int32 rowStart, int32 column);

	// Trigger checks that remove adjacent symbols and collapse symbols into empty spaces below.
	void TriggerRemoveCollapseAnimate();

	virtual void Tick(float DeltaTime) override;

protected:
	
	// Start the symbol collapse animation; call after animating the match removal.
	void AnimateCollapse();
	
	// Start the symbol removal animation; call after calling board remove matches at the end of a move.
	void AnimateRemoveMatches();

	virtual void BeginPlay() override;

	// Move symbols above empty spaces in the game board integer array down; return locations.
	TArray<FRowColumn> BoardCollapseEmpty();

	// Check if there are 3 or more of the same symbol adjacent to location and return direction.
	EDirections BoardCheckForAdjacentThree(int32 row, int32 column);

	// Generates the game board integer array - 0 empty, >0 symbol mesh indicies.
	UFUNCTION(BlueprintCallable)
	void BoardConstruct();

	// Remove adjacent matching symbols of 3 or more, and return locations.
	TArray<FRowColumn> BoardRemoveMatches();

	// Set the symbol in the board integer array at the row and column.
	void BoardSet(int32 row, int32 column, int32 symbol);

	// Construct the grid of static meshes that visually represent the game board symbols.
	UFUNCTION(BlueprintCallable)
	void SymbolMeshComponentsConstruct();

	// Replace the static mesh in the static mesh component at row, column.
	void SymbolMeshComponentsSet(int32 row, int32 column, int32 staticMeshIndex);
	
	// Stores the current state of animation for tick to look up.
	EAnimationState AnimationState = EAnimationState::IDLE;

	// The board value array, 0 for empty, and integer for symbol index.
	TArray<int32> Board;

	// The number of columns in the game board.
	int32 NumberOfColumns = GAME_BOARD_NUMBER_OF_COLUMNS;

	// The number of rows in the game board.
	int32 NumberOfRows = GAME_BOARD_NUMBER_OF_ROWS;

	// The grid spacing of the game board.
	float Spacing = GAME_BOARD_SPACING;

	// Material that symbols being removed get for a short time.
	UPROPERTY(EditAnywhere, Category = "GameBoard")
	UMaterial* SymbolRemovingMaterial;

	// Array of static mesh objects to use to display symbols on the game board.
	UPROPERTY(EditAnywhere, Category = "GameBoard")
	TArray<UStaticMesh*> SymbolStaticMeshArray;

	// Array of static mesh components show as the game state on the game board.
	TArray<UStaticMeshComponent*> SymbolStaticMeshComponents;

	// A place to store symbols being removed during the match calculations.
	TArray<FRowColumn> SymbolsToRemove;

	// A place to store symbols being collapsed (move downward).
	TArray<FRowColumn> SymbolsToCollapse;
};

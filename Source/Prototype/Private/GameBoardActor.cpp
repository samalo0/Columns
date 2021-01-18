// Copyright 2019
#include "GameBoardActor.h"

#include "Classes/Materials/Material.h"
#include "PrototypePawn.h"
#include "PrototypeGameModeBase.h"
#include "Components/StaticMeshComponent.h"

AGameBoardActor::AGameBoardActor()
{
	// Enable tick, but disable it for now; used for animation.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(false);
}

void AGameBoardActor::AnimateCollapse()
{
	// clear any symbols static mesh components that were highlighted to be removed
	for (auto index = 0; index < SymbolsToRemove.Num(); index++)
	{
		// find the static mesh component
		const int32 whichComponent = SymbolsToRemove[index].Row * GAME_BOARD_NUMBER_OF_COLUMNS + SymbolsToRemove[index].Column;
		if (whichComponent >= 0 && whichComponent < SymbolStaticMeshComponents.Num())
		{
			// clear the static mesh so it's invisible
			SymbolStaticMeshComponents[whichComponent]->SetStaticMesh(nullptr);
		}
	}
	
	// start up tick for collapse animation
	AnimationState = EAnimationState::EMPTY;
	PrimaryActorTick.SetTickFunctionEnable(true);
}

void AGameBoardActor::AnimateRemoveMatches()
{
	if (SymbolsToRemove.Num() > 0)
	{
		// if any symbols are being removed due to 3 or more adjacent
		for (auto index = 0; index < SymbolsToRemove.Num(); index++)
		{
			// find the static mesh component
			const int32 whichComponent = SymbolsToRemove[index].Row * GAME_BOARD_NUMBER_OF_COLUMNS + SymbolsToRemove[index].Column;
			if (whichComponent >= 0 && whichComponent < SymbolStaticMeshComponents.Num())
			{
				// set it's material to a highlight material
				SymbolStaticMeshComponents[whichComponent]->SetMaterial(0, SymbolRemovingMaterial);
			}
		}
			
		// start up tick to wait for symbols to hightlight for a short time
		AnimationState = EAnimationState::SYMBOLS;
		PrimaryActorTick.SetTickFunctionEnable(true);
	}
	else if (SymbolsToCollapse.Num() > 0)
	{
		AnimateCollapse();
	}
	else
	{
		// nothing to animate; reconstruct the component grid
		SymbolMeshComponentsConstruct();
		
		// trigger the pawn to start moving again and accept input
		APawn* playerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
		if (playerPawn != nullptr)
		{
			((APrototypePawn*)playerPawn)->TriggerNextMoveEnd();
		}
	}
}

void AGameBoardActor::BeginPlay()
{
	Super::BeginPlay();
}

TArray<FRowColumn> AGameBoardActor::BoardCollapseEmpty()
{
	bool changed = true;
	TArray<FRowColumn> locationsToMoveDown;

	// find symbols that are above empty spaces and move them down
	while (changed == true)
	{
		changed = false;

		for (auto row = 0; row < (NumberOfRows - 1); row++)
		{
			for (auto column = 0; column < NumberOfColumns; column++)
			{
				if (BoardGet(row, column) > 0 && BoardGet(row + 1, column) == 0)
				{
					BoardSet(row + 1, column, BoardGet(row, column));
					BoardSet(row, column, 0);
					changed = true;

					locationsToMoveDown.Add(FRowColumn(row, column));
				}
			}
		}
	}

	return locationsToMoveDown;
}

void AGameBoardActor::BoardConstruct()
{
	// empty out the board
	Board.Empty();

	// set up indexing for where to start the random symbols
	const int32 maxRowToStartSymbols = NumberOfRows - 2;
	const int32 minRowToStartSymbols = 4;

	// set up default place to start them, in case getting the game mode fails
	int32 rowToStartRandomSymbols = NumberOfRows - 2;

	APrototypeGameModeBase* gameMode = (APrototypeGameModeBase*)GetWorld()->GetAuthGameMode();
	if (gameMode != nullptr) rowToStartRandomSymbols = NumberOfRows - (gameMode->GetLevel() + 1);

	if (rowToStartRandomSymbols > maxRowToStartSymbols) rowToStartRandomSymbols = maxRowToStartSymbols;
	else if (rowToStartRandomSymbols < minRowToStartSymbols) rowToStartRandomSymbols = minRowToStartSymbols;

	// loop over empty portion
	for (auto row = 0; row < rowToStartRandomSymbols; row++)
	{
		for (auto column = 0; column < NumberOfColumns; column++)
		{
			Board.Add(0);
		}
	}

	// loop over random symbol portion
	for (auto row = rowToStartRandomSymbols; row < NumberOfRows; row++)
	{
		for (auto column = 0; column < NumberOfColumns; column++)
		{
			const int32 symbol = (rand() % SymbolStaticMeshArray.Num()) + 1;
			Board.Add(symbol);
		}
	}

	// now check for random symbols that are at least 3 adjacent, this is a
	// problem as the random symbols shouldn't auto-solve the puzzle
	for (auto row = rowToStartRandomSymbols; row < NumberOfRows; row++)
	{
		for (auto column = 0; column < NumberOfColumns; column++)
		{
			if (BoardCheckForAdjacentThree(row, column) != EDirections::INDETERMINATE)
			{
				// there are 3 adjacent; try replacing with a count down
				// this will result in a space if necessary
				for (int32 trySymbol = SymbolStaticMeshArray.Num(); trySymbol >= 0; trySymbol--)
				{
					BoardSet(row, column, trySymbol);
					if (BoardCheckForAdjacentThree(row, column) == EDirections::INDETERMINATE) break;
				}
			}
		}
	}
}

EDirections AGameBoardActor::BoardCheckForAdjacentThree(int32 row, int32 column)
{
	const int32 index = row * NumberOfColumns + column;

	// check that the index to check from exists
	if (index >= Board.Num())
	{
		return EDirections::INDETERMINATE;
	}

	// get the symbol to match against
	const int32 matchSymbol = Board[index];

	// check left
	if (column > 1)
	{
		if (BoardGet(row, column - 1) == matchSymbol
			&& BoardGet(row, column - 2) == matchSymbol)
			return EDirections::LEFT;
	}

	// check down left
	if (column > 1 && row < (NumberOfRows - 2))
	{
		if (BoardGet(row + 1, column - 1) == matchSymbol
			&& BoardGet(row + 2, column - 2) == matchSymbol)
			return EDirections::DOWN_LEFT;
	}

	// check down
	if (row < (NumberOfRows - 2))
	{
		if (BoardGet(row + 1, column) == matchSymbol
			&& BoardGet(row + 2, column) == matchSymbol)
			return EDirections::DOWN;
	}

	// check down right
	if (row < (NumberOfRows - 2) && column < (NumberOfColumns - 2))
	{
		if (BoardGet(row + 1, column + 1) == matchSymbol
			&& BoardGet(row + 2, column + 2) == matchSymbol)
			return EDirections::DOWN_RIGHT;
	}

	// check right
	if (column < (NumberOfColumns - 2))
	{
		if (BoardGet(row, column + 1) == matchSymbol
			&& BoardGet(row, column + 2) == matchSymbol)
			return EDirections::RIGHT;
	}

	// check up right
	if (row > 1 && column < (NumberOfColumns - 2))
	{
		if (BoardGet(row - 1, column + 1) == matchSymbol
			&& BoardGet(row - 2, column + 2) == matchSymbol)
			return EDirections::UP_RIGHT;
	}

	// check up
	if (row > 1)
	{
		if (BoardGet(row - 1, column) == matchSymbol
			&& BoardGet(row - 2, column) == matchSymbol)
			return EDirections::UP;
	}

	// check up left
	if (row > 1 && column > 1)
	{
		if (BoardGet(row - 1, column - 1) == matchSymbol
			&& BoardGet(row - 2, column - 2) == matchSymbol)
			return EDirections::UP_LEFT;
	}

	return EDirections::INDETERMINATE;
}

int32 AGameBoardActor::BoardGet(const int32 row, const int32 column) const
{
	const int32 index = row * NumberOfColumns + column;

	if (index >= 0 && index < Board.Num())
	{
		return Board[index];
	}

	// return no symbol if off the edge of the board
	return 0;
}

TArray<FRowColumn> AGameBoardActor::BoardRemoveMatches()
{
	bool anyMatches = false;

	TArray<FRowColumn> locationsToRemove;

	// loop over all symbols
	for (auto row = 0; row < NumberOfRows; row++)
	{
		for (auto column = 0; column < NumberOfColumns; column++)
		{
			// check the symbol at the current location; if empty, skip
			const int32 matchSymbol = BoardGet(row, column);
			if (matchSymbol == 0) continue;

			// get horizontal range of matches
			int32 horizStart = column;
			while (horizStart > 0 && BoardGet(row, horizStart - 1) == matchSymbol) horizStart--;

			int32 horizEnd = column;
			while (horizEnd < (NumberOfColumns - 1) && BoardGet(row, horizEnd + 1) == matchSymbol) horizEnd++;

			const int32 horizRangeCount = horizEnd - horizStart + 1;

			// get vertical range of matches
			int32 vertStart = row;
			while (vertStart > 0 && BoardGet(vertStart - 1, column) == matchSymbol) vertStart--;

			int32 vertEnd = row;
			while (vertEnd < (NumberOfRows - 1) && BoardGet(vertEnd + 1, column) == matchSymbol) vertEnd++;

			const int32 vertRangeCount = vertEnd - vertStart + 1;

			// get angled up right matches
			int32 upRight[2] = { row, column };
			while (upRight[0] > 0 && upRight[1] < (NumberOfColumns - 1) && BoardGet(upRight[0] - 1, upRight[1] + 1) == matchSymbol)
			{
				upRight[0]--;
				upRight[1]++;
			}

			int32 downLeft[2] = { row, column };
			while (downLeft[0] < (NumberOfRows - 1) && downLeft[1] > 0 && BoardGet(downLeft[0] + 1, downLeft[1] - 1) == matchSymbol)
			{
				downLeft[0]++;
				downLeft[1]--;
			}

			const int32 angledUpCount = upRight[1] - downLeft[1] + 1;

			// get angled down right matches
			int32 downRight[2] = { row, column };
			while (downRight[0] < (NumberOfRows - 1) && downRight[1] < (NumberOfColumns - 1)
				&& BoardGet(downRight[0] + 1, downRight[1] + 1) == matchSymbol)
			{
				downRight[0]++;
				downRight[1]++;
			}

			int32 upLeft[2] = { row, column };
			while (upLeft[0] > 0 && upLeft[1] > 0 && BoardGet(upLeft[0] - 1, upLeft[1] - 1) == matchSymbol)
			{
				upLeft[0]--;
				upLeft[1]--;
			}

			const int32 angledDownCount = downRight[1] - upLeft[1] + 1;

			// if any direction matched 3 or more, proceed to mark for removal
			if (horizRangeCount > 2 || vertRangeCount > 2 || angledUpCount > 2 || angledDownCount > 2)
			{
				anyMatches = true;

				// make an array of the objects to remove, starting with center
				locationsToRemove.Add(FRowColumn(row, column));

				if (horizRangeCount > 2)
				{
					for (auto cspan = horizStart; cspan <= horizEnd; cspan++) {
						// make sure to skip the center, it was added first
						if (cspan == column) continue;
						
						locationsToRemove.Add(FRowColumn(row, cspan));
					}
				}

				if (vertRangeCount > 2)
				{
					for (auto rspan = vertStart; rspan <= vertEnd; rspan++)
					{
						if (rspan == row) continue;
						
						locationsToRemove.Add(FRowColumn(rspan, column));
					}
				}

				if (angledUpCount > 2)
				{
					int32 rspan = downLeft[0];
					for (auto cspan = downLeft[1]; cspan <= upRight[1]; cspan++)
					{
						if (rspan == row && cspan == column) {
							rspan--;
							continue;
						}
						
						locationsToRemove.Add(FRowColumn(rspan, cspan));
						rspan--;
					}
				}

				if (angledDownCount > 2)
				{
					int32 rspan = upLeft[0];
					for (auto cspan = upLeft[1]; cspan <= downRight[1]; cspan++)
					{
						if (rspan == row && cspan == column) {
							rspan++;
							continue;
						}
						
						locationsToRemove.Add(FRowColumn(rspan, cspan));
						rspan++;
					}
				}
			}
		}
	}

	// remove any duplicates
	for (auto compareIndex = 0; compareIndex < locationsToRemove.Num() - 1; compareIndex++)
	{
		for (auto index = compareIndex + 1; index < locationsToRemove.Num(); index++)
		{
			if (locationsToRemove[compareIndex] == locationsToRemove[index])
			{
				locationsToRemove.RemoveAt(index);
				index--;
			}
		}
	}

	// remove symbols
	for (auto index = 0; index < locationsToRemove.Num(); index++)
	{
		BoardSet(locationsToRemove[index].Row, locationsToRemove[index].Column, 0);
	}

	return locationsToRemove;
}

void AGameBoardActor::BoardSet(int32 row, int32 column, int32 symbol)
{
	const int32 index = row * NumberOfColumns + column;

	if (index >= 0 && index < Board.Num() && symbol <= SymbolStaticMeshArray.Num())
	{
		Board[index] = symbol;
	}
}

void AGameBoardActor::BoardSetTrinity(TArray<int32> symbolsArray, int32 rowStart, int32 column)
{
	// add symbols to the game board; note symbols to add will be
	// indexed 0 to number of unique symbols - 1; need to add one
	// also replace the mesh of the components
	for (auto index = 0; index < symbolsArray.Num(); index++)
	{
		BoardSet(index + rowStart, column, symbolsArray[index] + 1);
		SymbolMeshComponentsSet(index + rowStart, column, symbolsArray[index]);
	}
}

void AGameBoardActor::SymbolMeshComponentsConstruct()
{
	// delete old symbols static mesh components first
	for (auto meshComponent = SymbolStaticMeshComponents.CreateIterator(); meshComponent; meshComponent++)
	{
		if (*meshComponent != nullptr) (*meshComponent)->DestroyComponent();
	}
	SymbolStaticMeshComponents.Empty();

	// register all to remove old ones
	RegisterAllComponents();

	// loop over the game board making the components
	int32 symbol = 1;
	for (auto row = 0; row < NumberOfRows; row++)
	{
		for (auto column = 0; column < NumberOfColumns; column++)
		{
			const FName uniqueName = FName(*FString::Printf(TEXT("Symbol%d"), symbol));
			UStaticMeshComponent* newComponent = NewObject<UStaticMeshComponent>(this, uniqueName);

			if (newComponent != nullptr)
			{
				SymbolStaticMeshComponents.Add(newComponent);
				newComponent->RegisterComponent();
				newComponent->SetRelativeLocation(FVector(column * Spacing, row * Spacing, 0.0f));
				newComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

				const int32 boardIndex = row * NumberOfColumns + column;
				if (boardIndex >= 0 && boardIndex < Board.Num())
				{
					const int32 symbolIndex = Board[boardIndex] - 1;

					if (symbolIndex >= 0 && symbolIndex < SymbolStaticMeshArray.Num())
					{
						newComponent->SetStaticMesh(SymbolStaticMeshArray[symbolIndex]);
					}
				}
			}

			symbol++;
		}
	}
}

void AGameBoardActor::SymbolMeshComponentsSet(int32 row, int32 column, int32 staticMeshIndex)
{
	const int32 index = row * NumberOfColumns + column;

	if (index >= 0 && index < SymbolStaticMeshComponents.Num() 
		&& staticMeshIndex >= 0 && staticMeshIndex < SymbolStaticMeshArray.Num())
	{
		SymbolStaticMeshComponents[index]->SetStaticMesh(SymbolStaticMeshArray[staticMeshIndex]);
	}
}

void AGameBoardActor::TriggerRemoveCollapseAnimate()
{
	// continue the symbol removal/collapse empty process
	APrototypeGameModeBase* gameMode = (APrototypeGameModeBase*)GetWorld()->GetAuthGameMode();

	if (gameMode != nullptr)
	{
		// collapse all matched symbols
		SymbolsToRemove = BoardRemoveMatches();

		// collapse all empties below symbols
		SymbolsToCollapse = BoardCollapseEmpty();

		// add to the score
		gameMode->AddToJewelsAndScore(SymbolsToRemove.Num());

		// trigger the game board to animate symbol removal/empty collapse, if any exist
		AnimateRemoveMatches();
	}
}

void AGameBoardActor::Tick(float DeltaTime)
{
	static float symbolTime = 0.0f;
	
	Super::Tick(DeltaTime);

	switch (AnimationState)
	{
	case EAnimationState::SYMBOLS:
		symbolTime += DeltaTime;
		if (symbolTime >= SYMBOL_HIGHLIGHT_SECONDS)
		{
			symbolTime = 0;
			if(SymbolsToCollapse.Num() > 0) AnimateCollapse();
			else
			{
				AnimationState = EAnimationState::IDLE;
				PrimaryActorTick.SetTickFunctionEnable(false);
				TriggerRemoveCollapseAnimate();
			}
		}
		break;
	case EAnimationState::EMPTY:
		for (auto index = 0; index < SymbolsToCollapse.Num(); index++)
		{
			// find the static mesh components to move
			const int32 whichComponent = SymbolsToCollapse[index].Row * GAME_BOARD_NUMBER_OF_COLUMNS + SymbolsToCollapse[index].Column;
			if (whichComponent >= 0 && whichComponent < SymbolStaticMeshComponents.Num())
			{
				// move them down
				SymbolStaticMeshComponents[whichComponent]->AddRelativeLocation(FVector(0.0f, DeltaTime * PAWN_SPEED_PIXELS_PER_SECOND, 0.0f));
				
				// if they've move one space, stop animation
				if (SymbolStaticMeshComponents[whichComponent]->GetRelativeTransform().GetLocation().Y >= (SymbolsToCollapse[index].Row + 1) * GAME_BOARD_SPACING)
				{
					// regenerate the symbol mesh components to match the board
					SymbolMeshComponentsConstruct();

					// stop tick, check if there's more symbols matching or more that need to move down
					AnimationState = EAnimationState::IDLE;
					PrimaryActorTick.SetTickFunctionEnable(false);
					TriggerRemoveCollapseAnimate();
					break;
				}
			}
		}
		break;
	}
}


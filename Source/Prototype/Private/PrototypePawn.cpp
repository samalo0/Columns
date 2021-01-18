// Copyright 2019
#include "PrototypePawn.h"

#include "Components/InputComponent.h"
#include "PrototypeGameModeBase.h"
#include "Components/StaticMeshComponent.h"
#include "GameBoardActor.h"

APrototypePawn::APrototypePawn()
{
	// Pawn will tick to move smoothly
	PrimaryActorTick.bCanEverTick = true;

	// Create a generate root component for the actor which will be stationary.
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>("RootSceneComponent");
	SetRootComponent(RootSceneComponent);
}

void APrototypePawn::BeginPlay()
{
	Super::BeginPlay();

	// create the first batch of symbols
	ConstructTrinity();

	// Set up the downward movement speed based on level.
	APrototypeGameModeBase *gameMode = (APrototypeGameModeBase *)GetWorld()->GetAuthGameMode();
	if (gameMode != nullptr)
	{
		GravityPixelsPerSecond = PAWN_GRAVITY_PIXELS_PER_SECOND + (gameMode->GetLevel() * PAWN_GRAVITY_LEVEL_SCALE);
	}
}

void APrototypePawn::ConstructTrinity() 
{
	// remove current symbol components
	for (auto oldComponents = CurrentPawnStaticMeshComponents.CreateIterator(); oldComponents; oldComponents++)
	{
		if ((*oldComponents) != nullptr) (*oldComponents)->DestroyComponent();
	}
	CurrentPawnStaticMeshComponents.Empty();
	CurrentSymbolIndicies.Empty();

	// remove next symbol components
	for (auto oldComponents = NextPawnStaticMeshComponents.CreateIterator(); oldComponents; oldComponents++)
	{
		if ((*oldComponents) != nullptr) (*oldComponents)->DestroyComponent();
	}
	NextPawnStaticMeshComponents.Empty();

	// remove them from the actor
	RegisterAllComponents();

	// reset location
	LocationX = 0;
	LocationY = -PAWN_SIZE;
	DesiredLocationX = 0;
	DesiredLocationY = -PAWN_SIZE;

	// if the next symbols have already been created, use them
	if (NextSymbolIndicies.Num() > 0)
	{
		CurrentSymbolIndicies = NextSymbolIndicies;
	}
	else
	{
		// otherwise just randomly generate them
		for (auto index = 0; index < PAWN_SIZE; index++)
		{
			const int32 symbolIndex = rand() % SymbolStaticMeshArray.Num();
			CurrentSymbolIndicies.Add(symbolIndex);
		}
	}
	NextSymbolIndicies.Empty();

	// construct the current trinity of symbols
	int32 stack = 1;
	for (auto index = 0; index < PAWN_SIZE; index++)
	{
		const FName uniqueName = FName(*FString::Printf(TEXT("CurrentMeshComponent%d"), stack++));
		UStaticMeshComponent* newComponent = NewObject<UStaticMeshComponent>(this, uniqueName);

		if (newComponent != nullptr)
		{
			CurrentPawnStaticMeshComponents.Add(newComponent);
			newComponent->RegisterComponent();
			newComponent->SetStaticMesh(SymbolStaticMeshArray[CurrentSymbolIndicies[index]]);

			if (index == 0)
			{
				newComponent->SetRelativeLocation(FVector(0.0f, LocationY * GAME_BOARD_SPACING, 0.0f));
				newComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			}
			else
			{
				newComponent->SetRelativeLocation(FVector(0.0f, index * GAME_BOARD_SPACING, 0.0f));
				newComponent->AttachToComponent(CurrentPawnStaticMeshComponents[0], FAttachmentTransformRules::KeepRelativeTransform);
			}
		}
	}

	// construct the next trinity of symbols
	stack = 1;
	for (auto index = 0; index < PAWN_SIZE; index++)
	{
		const FName uniqueName = FName(*FString::Printf(TEXT("NextMeshComponent%d"), stack++));
		UStaticMeshComponent* newComponent = NewObject<UStaticMeshComponent>(this, uniqueName);

		if (newComponent != nullptr)
		{
			NextPawnStaticMeshComponents.Add(newComponent);
			newComponent->RegisterComponent();
			newComponent->SetRelativeLocation(FVector(GAME_BOARD_SPACING * -4, (index + 2) * GAME_BOARD_SPACING, 0.0f));
			
			const int32 symbolIndex = rand() % SymbolStaticMeshArray.Num();
			newComponent->SetStaticMesh(SymbolStaticMeshArray[symbolIndex]);
			newComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

			NextSymbolIndicies.Add(symbolIndex);
		}
	}
}

void APrototypePawn::MoveDownPressed()
{
	if (GameBoardActor != nullptr)
	{
		// check if above the bottom row and there's room to move down
		if ((LocationY + PAWN_SIZE) < GAME_BOARD_NUMBER_OF_ROWS && GameBoardActor->BoardGet(LocationY + PAWN_SIZE, LocationX) == 0)
		{
			DesiredLocationY = LocationY + 1;
			MoveDownKeyHeldDown = true;
		}
	}
}

void APrototypePawn::MoveDownReleased()
{
	MoveDownKeyHeldDown = false;
}

void APrototypePawn::MoveLeft()
{
	// check if it's possible to move left
	if (GameBoardActor != nullptr)
	{
		if (LocationX > 0)
		{
			int32 row;
			for (row = LocationY; row < LocationY + PAWN_SIZE; row++)
			{
				if (GameBoardActor->BoardGet(row, LocationX - 1) != 0)	break;
			}

			if (row == (LocationY + PAWN_SIZE))
			{
				DesiredLocationX = LocationX - 1;
			}
		}
	}
}

void APrototypePawn::MoveRight()
{
	// check if it's possible to move right
	if (GameBoardActor != nullptr)
	{
		if (LocationX < (GAME_BOARD_NUMBER_OF_COLUMNS - 1))
		{
			int32 row;
			for (row = LocationY; row < LocationY + PAWN_SIZE; row++)
			{
				if (GameBoardActor->BoardGet(row, LocationX + 1) != 0)	break;
			}

			if (row == (LocationY + PAWN_SIZE))
			{
				DesiredLocationX = LocationX + 1;
			}
		}
	}
}

void APrototypePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	InputComponent->BindAction("MoveDown", EInputEvent::IE_Pressed, this, &APrototypePawn::MoveDownPressed);
	InputComponent->BindAction("MoveDown", EInputEvent::IE_Released, this, &APrototypePawn::MoveDownReleased);
	InputComponent->BindAction("MoveLeft", EInputEvent::IE_Pressed, this, &APrototypePawn::MoveLeft);
	InputComponent->BindAction("MoveRight", EInputEvent::IE_Pressed, this, &APrototypePawn::MoveRight);
	InputComponent->BindAction("ShuffleDown", EInputEvent::IE_Pressed, this, &APrototypePawn::ShuffleDown);
	InputComponent->BindAction("ShuffleUp", EInputEvent::IE_Pressed, this, &APrototypePawn::ShuffleUp);
}

void APrototypePawn::ShuffleDown()
{
	// shuffle the current symbols down
	if (CurrentSymbolIndicies.Num() == PAWN_SIZE && CurrentPawnStaticMeshComponents.Num() == PAWN_SIZE)
	{
		const int32 swap = CurrentSymbolIndicies.Last();
		CurrentSymbolIndicies.Insert(swap, 0);
		CurrentSymbolIndicies.RemoveAt(CurrentSymbolIndicies.Num() - 1);
		
		UStaticMesh *staticMeshHolder = CurrentPawnStaticMeshComponents.Last()->GetStaticMesh();
		for (auto index = CurrentPawnStaticMeshComponents.Num() - 1; index > 0; index--)
		{
			CurrentPawnStaticMeshComponents[index]->SetStaticMesh(CurrentPawnStaticMeshComponents[index - 1]->GetStaticMesh());
		}
		CurrentPawnStaticMeshComponents[0]->SetStaticMesh(staticMeshHolder);
	}
}

void APrototypePawn::ShuffleUp()
{
	// shuffle the current symbols up
	if (CurrentSymbolIndicies.Num() == PAWN_SIZE && CurrentPawnStaticMeshComponents.Num() == PAWN_SIZE)
	{
		const int32 swap = CurrentSymbolIndicies[0];
		CurrentSymbolIndicies.RemoveAt(0);
		CurrentSymbolIndicies.Add(swap);

		UStaticMesh* staticMeshHolder = CurrentPawnStaticMeshComponents[0]->GetStaticMesh();
		for (auto index = 0; index < PAWN_SIZE - 1; index++)
		{
			CurrentPawnStaticMeshComponents[index]->SetStaticMesh(CurrentPawnStaticMeshComponents[index + 1]->GetStaticMesh());
		}
		CurrentPawnStaticMeshComponents.Last()->SetStaticMesh(staticMeshHolder);
	}
}

void APrototypePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// check left/right movement
	if (DesiredLocationX > LocationX) {
		CurrentPawnStaticMeshComponents[0]->AddRelativeLocation(FVector(DeltaTime * PAWN_SPEED_PIXELS_PER_SECOND, 0.0f, 0.0f));
		if (CurrentPawnStaticMeshComponents[0]->GetRelativeTransform().GetLocation().X >= DesiredLocationX * GAME_BOARD_SPACING)
		{
			LocationX = DesiredLocationX;
			const float currentY = CurrentPawnStaticMeshComponents[0]->GetRelativeTransform().GetLocation().Y;
			CurrentPawnStaticMeshComponents[0]->SetRelativeLocation(FVector(LocationX * GAME_BOARD_SPACING, currentY, 0.0f));
		}
	}
	else if (DesiredLocationX < LocationX)
	{
		CurrentPawnStaticMeshComponents[0]->AddRelativeLocation(FVector(-DeltaTime * PAWN_SPEED_PIXELS_PER_SECOND, 0.0f, 0.0f));
		if (CurrentPawnStaticMeshComponents[0]->GetRelativeTransform().GetLocation().X <= DesiredLocationX * GAME_BOARD_SPACING)
		{
			LocationX = DesiredLocationX;
			const float currentY = CurrentPawnStaticMeshComponents[0]->GetRelativeTransform().GetLocation().Y;
			CurrentPawnStaticMeshComponents[0]->SetRelativeLocation(FVector(LocationX * GAME_BOARD_SPACING, currentY, 0.0f));
		}
	}

	// trigger speeder downward movement if down key held
	if (MoveDownKeyHeldDown && DesiredLocationY == LocationY)
	{
		DesiredLocationY++;
	}
	
	// check down movement
	if (DesiredLocationY > LocationY)
	{
		CurrentPawnStaticMeshComponents[0]->AddRelativeLocation(FVector(0.0f, DeltaTime * PAWN_SPEED_PIXELS_PER_SECOND, 0.0f));
		if (CurrentPawnStaticMeshComponents[0]->GetRelativeTransform().GetLocation().Y >= DesiredLocationY * GAME_BOARD_SPACING)
		{
			LocationY = DesiredLocationY;
			const float currentX = CurrentPawnStaticMeshComponents[0]->GetRelativeTransform().GetLocation().X;
			CurrentPawnStaticMeshComponents[0]->SetRelativeLocation(FVector(currentX, LocationY * GAME_BOARD_SPACING, 0.0f));
		}
	}
	else
	{
		// add gravity to automatically pull down
		CurrentPawnStaticMeshComponents[0]->AddRelativeLocation(FVector(0.0f, DeltaTime * GravityPixelsPerSecond, 0.0f));
		if (CurrentPawnStaticMeshComponents[0]->GetRelativeTransform().GetLocation().Y >= (LocationY + 1) * GAME_BOARD_SPACING)
		{
			// if it moves into the next space, update location
			LocationY++;
			const float currentX = CurrentPawnStaticMeshComponents[0]->GetRelativeTransform().GetLocation().X;
			CurrentPawnStaticMeshComponents[0]->SetRelativeLocation(FVector(currentX, LocationY * GAME_BOARD_SPACING, 0.0f));
		}
	}
	
	// check if downward collision
	if (GameBoardActor != nullptr)
	{
		// check if above the bottom row and there's room to move down
		if ((LocationY + PAWN_SIZE) >= GAME_BOARD_NUMBER_OF_ROWS || GameBoardActor->BoardGet(LocationY + PAWN_SIZE, LocationX) != 0)
		{
			TriggerNextMoveStart();
		}
	}
}

void APrototypePawn::TriggerNextMoveEnd()
{
	APrototypeGameModeBase* gameMode = (APrototypeGameModeBase*)GetWorld()->GetAuthGameMode();

	if (gameMode != nullptr)
	{
		// check for completion of the level
		if (gameMode->GetJewels() >= gameMode->GetJewelsRequired())
		{
			gameMode->TriggerNextLevel();
			return;
		}

		// re-enable movement and input
		PrimaryActorTick.SetTickFunctionEnable(true);
		EnableInput(GetWorld()->GetFirstPlayerController());
	}
}

void APrototypePawn::TriggerNextMoveStart()
{
	// get the game mode to add to score
	APrototypeGameModeBase* gameMode = (APrototypeGameModeBase*)GetWorld()->GetAuthGameMode();

	if (GameBoardActor != nullptr && gameMode != nullptr)
	{
		// disable tick and input while animating
		PrimaryActorTick.SetTickFunctionEnable(false);
		DisableInput(GetWorld()->GetFirstPlayerController());
		
		// disable held down key while input isn't available
		MoveDownKeyHeldDown = false;

		// check for game over
		if (LocationY < 0)
		{
			gameMode->TriggerEndGame();
			return;
		}
		
		// add the symbols to the board array, set static mesh components in the board
		GameBoardActor->BoardSetTrinity(CurrentSymbolIndicies, LocationY, LocationX);

		// construct the next set of board symbols (and move the trinity to not cover animations)
		ConstructTrinity();

		// trigger the repeat process that removes symbols, collapses empty spaces, and animates
		GameBoardActor->TriggerRemoveCollapseAnimate();
	}
}





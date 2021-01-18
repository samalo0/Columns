// Copyright 2019
#include "GridActor.h"

#include "Components/StaticMeshComponent.h"
#include "GameBoardActor.h"

AGridActor::AGridActor()
{
	// disable tick
	PrimaryActorTick.bCanEverTick = true;

	// get the static mesh from world or blueprint to use as the grid static mesh
	GridStaticMesh = CreateDefaultSubobject<UStaticMesh>(TEXT("GridStaticMesh"));
}

void AGridActor::BeginPlay()
{
	Super::BeginPlay();

	ConstructGrid();
}

void AGridActor::ConstructGrid()
{
	// clear out array if it exists
	for (auto gridComponent = GridMeshComponents.CreateIterator(); gridComponent; gridComponent++)
	{
		if((*gridComponent) != nullptr) (*gridComponent)->DestroyComponent();
	}
	GridMeshComponents.Empty();

	// update component registration to remove them from the world
	RegisterAllComponents();

	// loop over grid and generate
	int32 point = 1;
	for (auto row = 0; row < GAME_BOARD_NUMBER_OF_ROWS + 1; row++)
	{
		for (auto column = 0; column < GAME_BOARD_NUMBER_OF_COLUMNS + 1; column++)
		{
			// create a unique name - all components must have a unique name
			const FName uniqueName = FName(*FString::Printf(TEXT("GridPoint%d"), point++));

			UStaticMeshComponent* newComponent = NewObject<UStaticMeshComponent>(this, uniqueName);
			if (newComponent != nullptr)
			{
				GridMeshComponents.Add(newComponent);
				newComponent->SetStaticMesh(GridStaticMesh);
				newComponent->SetRelativeLocation(FVector(column * GAME_BOARD_SPACING, row * GAME_BOARD_SPACING, 0.0f));
				if (row == 0 && column == 0) SetRootComponent(newComponent);
				else newComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
				newComponent->RegisterComponent();
			}
		}
	}
}

void AGridActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


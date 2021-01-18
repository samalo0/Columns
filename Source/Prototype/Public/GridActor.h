// Copyright 2019
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridActor.generated.h"

UCLASS()
class PROTOTYPE_API AGridActor : public AActor
{

	GENERATED_BODY()
	
public:	
	
	AGridActor();
	
	// Construct the grid of static meshes for the play area.
	void ConstructGrid();

	virtual void Tick(float DeltaTime) override;

protected:
	
	virtual void BeginPlay() override;

	// Static mesh to use as the grid mesh.
	UPROPERTY(EditAnywhere, Category = "Grid")
	UStaticMesh* GridStaticMesh;
	
	// The array of static mesh components that make up the grid.
	TArray<UStaticMeshComponent*> GridMeshComponents;
	
};

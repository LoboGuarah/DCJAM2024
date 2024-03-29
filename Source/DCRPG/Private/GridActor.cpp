


#include "DCRPG/Public/GridActor.h"

#include "DCRPGCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DCRPG/Public/GridBlockerVolume.h"
#include "DCRPG/Public/GridData.h"
#include "Components/InstancedStaticMeshComponent.h"

// Sets default values
AGridActor::AGridActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AGridActor::SetActorHiddenInGame(true);
	InstancedStaticMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstanceStaticMeshComponent"));
	const UGridData* SetGridData = GridData.LoadSynchronous();
	if(SetGridData != nullptr)
	{
		InstancedStaticMeshComponent->SetStaticMesh(SetGridData->GetTileMesh().LoadSynchronous());
	}
}

// Called when the game starts or when spawned
void AGridActor::BeginPlay()
{
	Super::BeginPlay();

	RegenerateEnvironmentGrid();
	auto* PlayerCharacter = Cast<ADCRPGCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
	if(PlayerCharacter)
	{
		PlaceCharacterInGrid(PlayerCharacter->StartingGridPosition, PlayerCharacter);
	}
}

// Called every frame
void AGridActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGridActor::DestroyGrid()
{
	InstancedStaticMeshComponent->ClearInstances();
	GridIndexToInstanceIndex.Empty();
	TileDataMap.Empty();
}

void AGridActor::RegenerateEnvironmentGrid()
{
	if(GridData.IsNull())
	{
		UE_LOG(LogTemp,Warning, TEXT("No GridData set before spawning!"));
		return;
	}
	const auto* SetGridData = GridData.LoadSynchronous();
	DestroyGrid();
	InstancedStaticMeshComponent->SetStaticMesh(SetGridData->GetTileMesh().LoadSynchronous());
	
	const FIntPoint GridDimension = SetGridData->GetGridDimension();
	const float GridStep = InstancedStaticMeshComponent->GetStaticMesh()->GetBoundingBox().GetSize().X;
	for(int i = 0; i<GridDimension.X; i++)
	{
		for(int j = 0; j<GridDimension.Y; j++)
		{
			FVector TileLocation {GridStep * i, GridStep*j, 0};
			FTransform TileTransform;
			FVector NewLocation{};
			const bool bSpawnTile = TraceForGround(GetActorLocation() + TileLocation,NewLocation);
			if(bSpawnTile)
			{
				TileTransform = FTransform{NewLocation-GetActorLocation()+FVector{0,0,1}};
				AddTileAt(TileTransform, {i,j});
			}

		}
	}
}

bool AGridActor::TraceForGround(FVector TraceStartLocation, FVector& TraceHitLocation) const
{
	TArray<FHitResult> TraceHits{};
	constexpr float TraceRadius = 50.f;
	const bool bHit = UKismetSystemLibrary::SphereTraceMulti(GetWorld(), TraceStartLocation, TraceStartLocation-FVector{0,0,1000}, TraceRadius, UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), false, TArray<AActor*>{}, EDrawDebugTrace::None, TraceHits, true );
	if(bHit)
	{
		const auto* ModifierVolume = Cast<AGridBlockerVolume>(TraceHits[0].GetActor());
		if(ModifierVolume != nullptr)
		{
				return false;
		}
		TraceHitLocation = TraceHits[0].Location - FVector(0,0, TraceRadius);
	}
	return bHit;
}

bool AGridActor::ContainsTileWithIndex(const FIntPoint& TileIndex) const
{
	return GridIndexToInstanceIndex.Contains(TileIndex);
}

void AGridActor::AddTileAt(const FTransform& TileTransform, const FIntPoint& GridIndex)
{
	const int InstanceIndex = InstancedStaticMeshComponent->GetInstanceCount();
	UTileData* TileData = NewObject<UTileData>();
	TileData->SetInstanceIndex(InstanceIndex);
	GridIndexToInstanceIndex.Add(GridIndex, InstanceIndex);
	InstancedStaticMeshComponent->AddInstance(TileTransform);
	TileDataMap.Add(GridIndex,TileData);
}

bool AGridActor::RemoveTileAt(const FIntPoint& GridIndexToRemove)
{
	if(!GridIndexToInstanceIndex.Contains(GridIndexToRemove))
	{
		return false;
	}
	const int TargetIndex = GridIndexToInstanceIndex.FindAndRemoveChecked(GridIndexToRemove);
	TileDataMap.FindAndRemoveChecked(GridIndexToRemove);
	InstancedStaticMeshComponent->RemoveInstance(TargetIndex);
	return true;
}

void AGridActor::GetTileNeighborhood(const FIntPoint& TileIndex, TArray<FIntPoint>& OutNeighborhood, TMap<FIntPoint, UTileData*> TileSetData) const
{
	if(TileSetData.IsEmpty())
	{
		TileSetData = TileDataMap;
	}
	if(TileIndex.X <0 || TileIndex.Y <0 || !this->ContainsTileWithIndex(TileIndex))
	{
		UE_LOG(LogTemp,Warning, TEXT("Referenced grid does not contain a tile with the provided index."))
		return;
	}
	OutNeighborhood.Empty();
	//(X-1,Y), (X+1,Y), (X,Y-1), (X,Y+1)
	for(int i = -1;i <= 1; i++)
	{
		for(int j = -1;j <= 1; j++)
		{
			if(i == j || i == -j)
			{
				continue;
			}
			FIntPoint NeighborIndex {TileIndex.X+i,TileIndex.Y+j};
			if(TileSetData.Contains(NeighborIndex) && NeighborIndex != TileIndex)
			{
				OutNeighborhood.AddUnique(NeighborIndex);
			}
		}
	}
}

void AGridActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if(InstancedStaticMeshComponent->GetStaticMesh() != nullptr)
	{
		InstancedStaticMeshComponent->GetStaticMesh()->SetMaterial(0, GridData.LoadSynchronous()->GetTileBorderMaterial().LoadSynchronous());
	}
}


bool AGridActor::IsPositionOccupied(const FIntPoint& Position) const
{
	if(!ContainsTileWithIndex(Position))
	{
		UE_LOG(LogTemp, Warning, TEXT("Checked occupation of inexistant grid tile"))
		return false;
	}
	return TileDataMap.FindChecked(Position)->IsTileOccupied();
}

void AGridActor::PlaceCharacterInGrid(const FIntPoint& TargetTile, ADCRPGCharacter* Character)
{
	if(!ContainsTileWithIndex(TargetTile))
	{
		UE_LOG(LogTemp,Warning, TEXT("Tried to place character at invalid grid tile."))
		return;
	}
	auto* TileData = TileDataMap.FindChecked(TargetTile);
	TileData->SetOccupantCharacter(Character);
	Character->CurrentGridPosition = TargetTile;
	const FVector TileLocation = GetTileLocationByGridPosition(TargetTile);
	Character->InhabitGrid(this);
	Character->SetActorLocation(FVector(TileLocation.X, TileLocation.Y, TileLocation.Z + Character->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()),false,nullptr, ETeleportType::TeleportPhysics);

}

FVector AGridActor::GetTileLocationByGridPosition(const FIntPoint& GridPosition)
{
	if(!ContainsTileWithIndex(GridPosition))
	{
		UE_LOG(LogTemp,Warning, TEXT("Requested position for inexistant tile"))
		return FVector::ZeroVector;
	}
	FTransform TileTransform;
	const auto* TileData = TileDataMap.FindChecked(GridPosition);
	InstancedStaticMeshComponent->GetInstanceTransform(TileData->GetInstanceIndex(), TileTransform, true);
	return TileTransform.GetLocation();
}

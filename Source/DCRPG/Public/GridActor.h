#pragma once

#include "CoreMinimal.h"
#include <GameFramework/Actor.h>
#include "GridActor.generated.h"

class ADCRPGCharacter;

USTRUCT()
struct FPathData
{
	GENERATED_BODY()
private:
	
	TOptional<FIntPoint> Connection {};

	UPROPERTY(VisibleAnywhere)
	int G{INT_MAX};
	
	UPROPERTY(VisibleAnywhere)
	int H{0};

	friend class UTileData;
};

UCLASS()
class UTileData : public UObject
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	int InstanceIndex;
	
	UPROPERTY(VisibleAnywhere)
	FPathData TilePathData;

	UPROPERTY(VisibleAnywhere)
	APawn* OccupantCharacter {nullptr};

public:
	[[nodiscard]] int GetInstanceIndex() const
	{
		return InstanceIndex;
	}

	void SetInstanceIndex(int NewIndex)
	{
		this->InstanceIndex = NewIndex;
	}

	




	[[nodiscard]] const FPathData& GetTilePathData() const
	{
		return TilePathData;
	}

	void SetTilePathData(const FPathData& InTilePathData)
	{
		this->TilePathData = InTilePathData;
	}

	[[nodiscard]] APawn* GetOccupantCharacter() const
	{
		return OccupantCharacter;
	}

	void SetOccupantCharacter(APawn* InOccupantCharacter)
	{
		this->OccupantCharacter = InOccupantCharacter;
	}

	TOptional<FIntPoint>& GetConnectedTile() {return TilePathData.Connection;}
	void SetConnectedTile(const FIntPoint& InConnection) {TilePathData.Connection = InConnection;}
	void ResetConnectedTile() {TilePathData.Connection.Reset();}
	void SetGValue(const int32 InValue) {TilePathData.G = InValue;}
	int& GetGValue() {return TilePathData.G;}
	void SetHValue(const int32 InValue) {TilePathData.H = InValue;}
	int& GetHValue() {return TilePathData.H;}
	int GetFValue() const {return TilePathData.G + TilePathData.H;}
	bool IsTileOccupied() const { return OccupantCharacter != nullptr;}

};
UCLASS()
class DCRPG_API AGridActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGridActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<class UGridData> GridData{nullptr};

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	TObjectPtr<class UInstancedStaticMeshComponent> InstancedStaticMeshComponent{nullptr};

	TMap<FIntPoint, int> GridIndexToInstanceIndex{};
	
	UPROPERTY(VisibleInstanceOnly)
	TMap<FIntPoint, UTileData*> TileDataMap{};

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(CallInEditor, Category = "Debug Utilities")
	void DestroyGrid();
	UFUNCTION(CallInEditor, Category = "Debug Utilities")
	void RegenerateEnvironmentGrid();

	UPROPERTY(EditInstanceOnly)
	FIntPoint PlayerStartingPosition {0};

	
	UFUNCTION(BlueprintCallable)
	bool TraceForGround(FVector TraceStartLocation, FVector& TraceHitLocation) const;

	bool ContainsTileWithIndex(const FIntPoint& TileIndex) const;
	
	void AddTileAt(const FTransform& TileTransform, const FIntPoint& GridIndex);
	bool RemoveTileAt(const FIntPoint& GridIndexToRemove);

	void GetTileNeighborhood(const FIntPoint& TileIndex, TArray<FIntPoint>& OutNeighborhood, TMap<FIntPoint, UTileData*> TileSetData = {}) const;

	virtual void OnConstruction(const FTransform& Transform) override;

	bool IsPositionOccupied(const FIntPoint& Position) const;
	
	void PlaceCharacterInGrid(const FIntPoint& TargetTile, ADCRPGCharacter* Character);

	UFUNCTION(BlueprintCallable)
	FVector GetTileLocationByGridPosition(const FIntPoint& GridPosition);
};



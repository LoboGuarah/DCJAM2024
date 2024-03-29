// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GridData.generated.h"

/**
 * 
 */
UCLASS()
class DCRPG_API UGridData : public UDataAsset
{
	GENERATED_BODY()
public:

	const FIntPoint& GetGridDimension() const
	{
		return GridDimension;
	}
	
	const TSoftObjectPtr<UStaticMesh>& GetTileMesh() const
	{
		return TileMesh;
	}
	
	const TSoftObjectPtr<UMaterialInstance>& GetTileBorderMaterial() const
	{
		return TileBorderMaterial;
	}
private:
	UPROPERTY(EditDefaultsOnly)
	FIntPoint GridDimension{1,1};

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UStaticMesh> TileMesh{nullptr};

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UMaterialInstance> TileBorderMaterial{nullptr};
};

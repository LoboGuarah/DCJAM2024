#include "DCRPG/Public/GridBlockerVolume.h"

AGridBlockerVolume::AGridBlockerVolume()
{
	AGridBlockerVolume::SetActorHiddenInGame(true);
	GetStaticMeshComponent()->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(FLinearColor::Red) );
}

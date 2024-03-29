// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DCRPGCharacter.generated.h"

class UInterpToMovementComponent;
class AGridActor;

// UENUM()
// enum EMovementDirection
// {
// 	None,
// 	Forward,
// 	Backwards,
// 	Left,
// 	Right
// };

UCLASS()
class DCRPG_API ADCRPGCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADCRPGCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FRotator StartRotation;
	FRotator DesiredRotation;
	FVector DesiredPosition;
	FVector MovementStartPosition;
	bool bIsMoving {false};

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void AssignGrid(AGridActor* InGrid) { InhabitedGrid = InGrid; OnGridAssigned();}
	FIntPoint CurrentGridPosition {0};

	UPROPERTY(EditInstanceOnly)
	FIntPoint StartingGridPosition {0};

	UPROPERTY(EditDefaultsOnly)
	int GridStepSize {100};
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	inline void InhabitGrid(AGridActor* InGrid){ InhabitedGrid = InGrid; }
private:
	UPROPERTY(EditInstanceOnly)
	AGridActor* InhabitedGrid {nullptr};
	
	void OnGridAssigned();
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Rotate(bool bClockwise);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool TryMove(bool bForward);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool TryStrafe(bool bRight);
};

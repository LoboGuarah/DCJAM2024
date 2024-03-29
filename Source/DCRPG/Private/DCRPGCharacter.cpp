// Fill out your copyright notice in the Description page of Project Settings.


#include "DCRPGCharacter.h"
#include "GridActor.h"
#include "Components/InterpToMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
ADCRPGCharacter::ADCRPGCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

// Called when the game starts or when spawned
void ADCRPGCharacter::BeginPlay()
{
	Super::BeginPlay();

	DesiredRotation = GetActorRotation();
}

// Called every frame

void ADCRPGCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(bIsMoving)
	{
		if(InhabitedGrid != nullptr)
		{
			FVector TargetLocation = InhabitedGrid->GetTileLocationByGridPosition(CurrentGridPosition);
			const FVector CurrentLocation = GetActorLocation();
			TargetLocation = FVector(TargetLocation.X, TargetLocation.Y, CurrentLocation.Z);
#if WITH_EDITOR
			GEngine->AddOnScreenDebugMessage(0, 5, FColor::Purple, FString::Format(TEXT("Actor Location: {0}, {1}, {2}, Actor Rotation: {3}, {4}, {5}"), {CurrentLocation.X, CurrentLocation.Y, CurrentLocation.Z, GetActorRotation().Roll, GetActorRotation().Pitch, GetActorRotation().Yaw}));
			GEngine->AddOnScreenDebugMessage(1, 5, FColor::Purple, FString::Format(TEXT("Target Location: {0}, {1}, {2}, Target Rotation: {3}, {4}, {5}"), {TargetLocation.X, TargetLocation.Y, TargetLocation.Z, DesiredRotation.Roll, DesiredRotation.Pitch, DesiredRotation.Yaw}));
#endif
			if(!DesiredRotation.Equals(GetActorRotation(), .1f))
			{
				SetActorRotation(FMath::RInterpTo(GetActorRotation(), DesiredRotation, DeltaTime, 3.f));
			}
			else if(!TargetLocation.Equals(CurrentLocation, 10))
			{
				SetActorLocation(FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaTime, 3.f));
			}
			else
			{
				SetActorRotation(DesiredRotation);
				SetActorLocation(TargetLocation);
				bIsMoving = false;
			}
		}
		
	}
}
// Called to bind functionality to input
void ADCRPGCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ADCRPGCharacter::OnGridAssigned()
{
	if(IsPlayerControlled())
	{
		InhabitedGrid->PlaceCharacterInGrid(StartingGridPosition, this);
	}
}

void ADCRPGCharacter::Rotate(bool bClockwise)
{
	if(!bIsMoving)
	{
		StartRotation = GetActorRotation();
		bIsMoving = true;
		const FRotator RotateTo = bClockwise ? FRotator(0, 90.0, 0) : FRotator(0, -90.0, 0);
		DesiredRotation = UKismetMathLibrary::ComposeRotators(GetActorRotation(), RotateTo);
	}
}
#pragma optimize("",off)
bool ADCRPGCharacter::TryMove(bool bForward)
{
	if(InhabitedGrid == nullptr)
	{
		UE_LOG(LogTemp,Warning, TEXT("Character tried to move but is not in a grid"));
		return false;
	}
	if(bIsMoving)
	{
		return false;
	}
	
	FIntPoint GoalDirection;
	auto DotProduct = UKismetMathLibrary::Dot_VectorVector(GetActorForwardVector(), InhabitedGrid->GetActorForwardVector());
	if(UKismetMathLibrary::Abs(DotProduct) <= 0.01) // Direction: Actor Forward is Grid Right
	{
		if( (GetActorForwardVector() + InhabitedGrid->GetActorRightVector()).Equals(FVector::ZeroVector,.01f)) // Facing Negative
		{
			bForward ? GoalDirection= {0,-1} : GoalDirection= {0,1};				
		}
		else
		{
			bForward ? GoalDirection= {0,1} : GoalDirection= {0,-1};
		}
	}
	else //Actor Forward is Grid Forward
	{
		if( (GetActorForwardVector() + InhabitedGrid->GetActorForwardVector()).Equals(FVector::ZeroVector, .01f)) // Facing Negative
		{
			bForward ? GoalDirection= {-1,0} : GoalDirection= {1,0};
		}
		else
		{
			bForward ? GoalDirection= {1,0} : GoalDirection= {-1,0};
		}
	}
	if(InhabitedGrid->ContainsTileWithIndex(CurrentGridPosition+GoalDirection))
	{
		MovementStartPosition = GetActorLocation();
		bIsMoving = true;
		CurrentGridPosition += GoalDirection;
		return true;
	}
	return false;
}

bool ADCRPGCharacter::TryStrafe(bool bRight)
{
	if(InhabitedGrid == nullptr)
	{
		UE_LOG(LogTemp,Warning, TEXT("Character tried to move but is not in a grid"));
		return false;
	}
	if(bIsMoving)
	{
		return false;
	}
	
	FIntPoint GoalDirection;
	auto DotProduct = UKismetMathLibrary::Dot_VectorVector(GetActorForwardVector(), InhabitedGrid->GetActorForwardVector());
	if(UKismetMathLibrary::Abs(DotProduct) <= 0.01) // Direction: Actor Forward is Grid Right
	{
		if( (GetActorForwardVector() + InhabitedGrid->GetActorRightVector()).Equals(FVector::ZeroVector,.01f)) // Facing Negative
		{
			bRight ? GoalDirection= {1,0} : GoalDirection= {-1,0};				
		}
		else
		{
			bRight ? GoalDirection= {-1,0} : GoalDirection= {1,0};
		}
	}
	else //Actor Forward is Grid Forward
	{
		if( (GetActorForwardVector() + InhabitedGrid->GetActorForwardVector()).Equals(FVector::ZeroVector,.01f)) // Facing Negative
		{
			bRight ? GoalDirection= {0,-1} : GoalDirection= {0,1};
		}
		else
		{
			bRight ? GoalDirection= {0,1} : GoalDirection= {0,-1};
		}
	}
	if(InhabitedGrid->ContainsTileWithIndex(CurrentGridPosition+GoalDirection))
	{
		MovementStartPosition = GetActorLocation();
		bIsMoving = true;
		CurrentGridPosition += GoalDirection;
		return true;
	}
	return false;
}
#pragma optimize("",on)

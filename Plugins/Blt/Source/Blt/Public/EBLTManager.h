// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Navigation/PathFollowingComponent.h"
#include "EBLTManager.generated.h"




class EBLTTestingStrategy
{
public:
	
};

UCLASS()
class EBLT_API AEBLTManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEBLTManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent)
	void RunPathfindingTest(AActor* actor, AActor* location);

	void RunPathfindingTest_Implementation(AActor* actor, AActor* location){}

	UFUNCTION(BlueprintImplementableEvent)
	void OnMoveCompletedEvent(FAIRequestID RequestID, EPathFollowingResult::Type Result);

	void OnMoveCompletedEvent_Implementation(FAIRequestID RequestID, EPathFollowingResult::Type Result);
};

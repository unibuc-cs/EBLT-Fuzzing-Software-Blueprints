// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TestAnnotation.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Navigation/PathFollowingComponent.h"
#include "EBLTCommonUtils.h"
#include "EBLTManager.generated.h"


class AEBLTTestTemplate;
class UBlueprintGeneratedClass;

class EBLTTestingStrategy
{
public:
	
};

UENUM(BlueprintType)
enum class EBLTManagerState : uint8 // \describes the lifecycle of a blueprint test
{
	EBLT_Idle, // Not doing anything
	EBLT_RunningSuite, // Tests are in progress
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

	void GetTestsToRun(TArray<FString>& outTestsToRun, TestParamsSuggestionStrategy& outTestStrategy) const;

	MapFromTestNameToAnnotations m_testNamesToAnnotations;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Triggers")
	void RunTestSuite();

	UFUNCTION(BlueprintCallable, Category = "Triggers")
	void InitTestsSuite();

	UFUNCTION(BlueprintNativeEvent)
	void RunPathfindingTest(AActor* actor, AActor* location);

	void RunPathfindingTest_Implementation(AActor* actor, AActor* location){}

	UFUNCTION(BlueprintImplementableEvent)
	void OnMoveCompletedEvent(FAIRequestID RequestID, EPathFollowingResult::Type Result);

	void OnTestFinished(AEBLTTestTemplate* testWhoFinished, const EBLTTestStatus finishedStatus);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (RelativePath))
	FFilePath AnnotationsPath;

	void OnMoveCompletedEvent_Implementation(FAIRequestID RequestID, EPathFollowingResult::Type Result);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (RelativePath))
	TArray<UBlueprintGeneratedClass*> AllowedTestInstances;

	UPROPERTY(BlueprintReadWrite, Category = "Triggers")
	bool m_continuousTestRunning = false;

	EBLTManagerState m_state = EBLTManagerState::EBLT_Idle;
	TSet<AEBLTTestTemplate*> m_currentlyRunningTests;

};

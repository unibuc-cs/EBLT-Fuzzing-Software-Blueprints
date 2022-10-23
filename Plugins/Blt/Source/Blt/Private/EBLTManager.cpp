// Fill out your copyright notice in the Description page of Project Settings.


#include "EBLTManager.h"
#include "EngineUtils.h"
#include "Misc/OutputDeviceNull.h"
#include "EBltBPLibrary.h"
#include "EBLTTestTemplate.h"

#pragma optimize("", off)

// Sets default values
AEBLTManager::AEBLTManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}
 
// Called when the game starts or when spawned
void AEBLTManager::BeginPlay()
{
	Super::BeginPlay();

	InitTestsSuite();
}
 
// Called every frame
void AEBLTManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	bool runTestSuite = false;
	if (runTestSuite)
	{
		RunTestSuite();
	}


	bool runTest = false;
	if (runTest)
	{
		runTest = false;
		
		AActor* targetDestActor = nullptr;

		//in cpp
		for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			if (ActorItr->GetName().Contains(("BP_Checkpoint2")))
			{
				targetDestActor = *ActorItr;
			}
		}

		AActor* targetTestActor = nullptr;

		//in cpp
		for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			if (ActorItr->GetName().Contains(("NPC_EpicCharacter")))
			{
				targetTestActor = *ActorItr;
			}
		}

		//FOutputDeviceNull ar;
		//const FString command = FString::Printf(TEXT("Test_PathfindingPathStr %s"), *targetDestActor->GetName());
		//this->CallFunctionByNameWithArguments(*command, ar, NULL, true);

		//RunPathfindingTest(targetDestActor, targetTestActor);
	}
}


void AEBLTManager::OnMoveCompletedEvent_Implementation(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	int a = 3;
	a++;
}

void AEBLTManager::RunTestSuite()
{
	// Spawn test actors according to the strategy and stuff
	// TODO

	// For each selected test set an instance with parameter and run it
	for (TPair<FString, SingleTestAnnotations>& testData : m_testNamesToAnnotations)
	{
		const FString& testName = testData.Key;
		SingleTestAnnotations& testSpecs = testData.Value;

		//
		TestsAnnotationsHelper::BuildTestInstance(GetWorld(), TestParamsSuggestionStrategy::TESTPARAMSTRATEGY_RANDOM, 
												testSpecs.m_spawnedTestActorForTest, testSpecs);

	}
	//


	// Wait for tests to complete
	// TODO
}

void AEBLTManager::InitTestsSuite()
{
	m_testNamesToAnnotations.Empty();

	// Step 1: parse the annotations and make internal setups
	TestsAnnotationsHelper::ParseTestsAnnotationsFromJSon(AnnotationsPath.FilePath, m_testNamesToAnnotations);

	// Step 2: Spawn actors for all tests in the level - might not be a very good idea in practice
	// TODO: what if too many ? maybe certain levels need to focus on particular tests ! We could also address this by multiple instances reading different annotations files

	for (TPair<FString, SingleTestAnnotations>& testData : m_testNamesToAnnotations)
	{
		const FString& testName = testData.Key;
		SingleTestAnnotations& testSpecs = testData.Value;

		AActor* testActor = GetWorld()->SpawnActor(testSpecs.m_classToTest);
		ensureMsgf(testActor, TEXT("couldnt spawn the testing actor"));
		testSpecs.m_spawnedTestActorForTest = testActor;
	}
}


#pragma optimize("", on)

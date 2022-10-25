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


	// Do not it idle :)
	// TODO: clusterize this operation, put it on VMs with Docker, etc ! See the tasks list
	if (m_state == EBLTManagerState::EBLT_Idle)
	{
		RunTestSuite();
	}
}


void AEBLTManager::OnMoveCompletedEvent_Implementation(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	int a = 3;
	a++;
}

void AEBLTManager::RunTestSuite()
{
	UE_LOG(LogBlt, Warning, TEXT("=========== RUNNING A NEW TEST SUITE ==========="));

	ensureMsgf(m_currentlyRunningTests.IsEmpty(), TEXT("You are currently trying to run a new test suite but existing one is in progres !!!!"));

	// Step 0: get a set of tests to select and the strategy used for fuzzing input variables
	TArray<FString> selectedTests;
	TestParamsSuggestionStrategy testStrategy = TestParamsSuggestionStrategy::TESTPARAMSTRATEGY_RANDOM;
	GetTestsToRun(selectedTests, testStrategy);

	// For each selected test set an instance with parameter and run it
	for (const FString& selectedTestName : selectedTests)
	{
		SingleTestAnnotations& testSpecs = m_testNamesToAnnotations[selectedTestName];

		TestsAnnotationsHelper::BuildTestInstance(GetWorld(), testStrategy, testSpecs.m_spawnedTestActorForTest, testSpecs);
		m_currentlyRunningTests.Add(CastChecked<AEBLTTestTemplate>(testSpecs.m_spawnedTestActorForTest));
	}


	m_state = EBLTManagerState::EBLT_RunningSuite;
}

// Init all tests basically
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
		AEBLTTestTemplate* testTemplateActor = Cast<AEBLTTestTemplate>(testSpecs.m_spawnedTestActorForTest);
		testTemplateActor->SetTestAnnotations(&testSpecs);
		testTemplateActor->SetGivenName(testName);
	}
}

void AEBLTManager::GetTestsToRun(TArray<FString>& outTestsToRun, TestParamsSuggestionStrategy& outTestingStrategy) const
{
	// TODO: see tasks list !
	// 1: strategy pattern to select by reading properties
	// 2: this need to be a flask process deployed in a different process.
	// 3: as a demo app, just run all tests / random for now
	outTestsToRun.Empty();

	for (const auto& it : m_testNamesToAnnotations)
	{
		outTestsToRun.Add(it.Key);
	}

	// 4: get a strategy and simulator instance to use too !
	outTestingStrategy = TestParamsSuggestionStrategy::TESTPARAMSTRATEGY_RANDOM;
}

void AEBLTManager::OnTestFinished(AEBLTTestTemplate* testWhoFinished, const EBLTTestStatus finishedStatus)
{
	UE_LOG(LogBlt, Warning, TEXT(" ## Test %s was finished"), *testWhoFinished->GetGivenName());

	// Remove the test actor from the set of running and destroy it
	m_currentlyRunningTests.Remove(testWhoFinished);
	testWhoFinished->Destroy();


	// If currently running tests is emtpy move back to idle
	if (m_currentlyRunningTests.Num() == 0)
	{
		if (m_continuousTestRunning)
		{
			m_state = EBLTManagerState::EBLT_Idle;
		}
	}
	
}


#pragma optimize("", on)

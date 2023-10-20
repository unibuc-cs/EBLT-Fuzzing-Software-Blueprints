// Fill out your copyright notice in the Description page of Project Settings.


#include "EBLTManager.h"
#include "EngineUtils.h"
#include "Misc/OutputDeviceNull.h"
#include "EBltBPLibrary.h"
#include "EBLTTestTemplate.h"
#include "Interfaces/IPluginManager.h"
#include "GameFramework/Character.h"

PRAGMA_DISABLE_OPTIMIZATION

AEBLTManager* AEBLTManager::m_instance = nullptr;
UWorld* AEBLTManager::m_myWorld = nullptr;

// Sets default values
AEBLTManager::AEBLTManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_EBLTContentDir = IPluginManager::Get().FindPlugin(TEXT("EBLT"))->GetContentDir();
}

AEBLTManager* AEBLTManager::getInstance()
{
	if (m_instance == nullptr)
	{
		ensureMsgf(m_instance, TEXT("In this version we require the manager to be added to a level"));
		//m_instance = new AEBLTManager();
	}
	return m_instance;
}

UWorld* AEBLTManager::getMyWorld()
{
	return m_instance->GetWorld();
}

 
// Called when the game starts or when spawned
void AEBLTManager::BeginPlay()
{
  	Super::BeginPlay();


	ensureMsgf(m_instance == nullptr, TEXT("THis object already exists"));
	m_instance = this;
	m_myWorld = GetWorld();

	//InitTestsSuite();
}

void AEBLTManager::Destroyed()
{
	//ensureMsgf(m_instance, TEXT("THis object was not registered"));
	m_instance = nullptr;

	UEBltBPLibrary::OnEBLTManagerDestroyed();
}

void AEBLTManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ensureMsgf(m_instance, TEXT("THis object was not registered"));
	m_instance = nullptr;
	m_myWorld = nullptr;

	UEBltBPLibrary::OnEBLTManagerDestroyed();
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
	if (m_testNamesToAnnotations.Num() <= 0)
	{
		return; 
	}

	UE_LOG(LogBlt, Warning, TEXT("=========== RUNNING A NEW TEST SUITE ==========="));

	ensureMsgf(m_currentlyRunningTests.IsEmpty(), TEXT("You are currently trying to run a new test suite but existing one is in progres !!!!"));

	// Step 0: get a set of tests to select and the strategy used for fuzzing input variables
	m_state_SelectedTests.Reset();
	m_state_testStrategy = TestParamsSuggestionStrategy::TESTPARAMSTRATEGY_RANDOM;
	GetTestsToRun(m_state_SelectedTests, m_state_testStrategy);

	if (m_state_SelectedTests.Num() > 0)
	{
		m_nextTestToRun_TestIndex_And_InstanceIndex.Key = 0;
		m_nextTestToRun_TestIndex_And_InstanceIndex.Value = -1;

		const bool hasNextTest = internal_getNextTestToRun();

		if (hasNextTest)
		{
			m_state = EBLTManagerState::EBLT_RunningSuite;
		}
	}
}

bool AEBLTManager::internal_getNextTestToRun()
{
	ensure(m_nextTestToRun_TestIndex_And_InstanceIndex.Key >= 0 && m_nextTestToRun_TestIndex_And_InstanceIndex.Key < m_state_SelectedTests.Num());
	
	// Increase the instance index
	m_nextTestToRun_TestIndex_And_InstanceIndex.Value++;

	// If at the end for the current test, go to next test
	if (m_testNamesToAnnotations[m_state_SelectedTests[m_nextTestToRun_TestIndex_And_InstanceIndex.Key]].m_numInstancesToRun <= m_nextTestToRun_TestIndex_And_InstanceIndex.Value)
	{
		m_nextTestToRun_TestIndex_And_InstanceIndex.Key++;

		if (m_nextTestToRun_TestIndex_And_InstanceIndex.Key >= m_state_SelectedTests.Num())
		{
			// Finished
			return false;
		}

		// Init to the first instance !
		m_nextTestToRun_TestIndex_And_InstanceIndex.Value = 0; 
	}

	// For each selected test set an instance with parameter and run it
	const SingleTestAnnotations& testSpecs = m_testNamesToAnnotations[m_state_SelectedTests[m_nextTestToRun_TestIndex_And_InstanceIndex.Key]];
	TestsAnnotationsHelper::BuildTestInstance(GetWorld(), m_state_testStrategy, testSpecs.m_spawnedTestActorForTest, testSpecs);
	m_currentlyRunningTests.Add(CastChecked<AEBLTTestTemplate>(testSpecs.m_spawnedTestActorForTest));

	return true;
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

		// Spawn a test instance based on provided blueprint
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
	ensure(finishedStatus == EBLTTestStatus::EBLTTest_Success || finishedStatus == EBLTTestStatus::EBLTTest_Failed);
	UE_LOG(LogBlt, Warning, TEXT(" ## Test %s, instance %d, was finished with code %s"), *testWhoFinished->GetGivenName(), m_nextTestToRun_TestIndex_And_InstanceIndex.Value, EBLTCommonUtils::EnumToString(finishedStatus));

	const bool isTesting = testWhoFinished->m_EBLTType == EBLTType::EBLT_FuzzForTesting;


	if (GEngine)
	{
		FColor colorForDebugText = FColor::Black;
		if (isTesting)
		{
			colorForDebugText = (finishedStatus == EBLTTestStatus::EBLTTest_Success ? FColor::Green : FColor::Red);
		}
		else
		{
			colorForDebugText = (finishedStatus == EBLTTestStatus::EBLTTest_Success ? FColor::Blue : FColor::Yellow);
		}


		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 1000, colorForDebugText, FString::Printf(TEXT(" ## %s %s, instance %d,  was finished - %s.%s"),
			(isTesting ? TEXT("TEST - ") : TEXT("TUNE - ")),
			*testWhoFinished->GetGivenName(),
			m_nextTestToRun_TestIndex_And_InstanceIndex.Value,
			EBLTCommonUtils::EnumToString(finishedStatus), 
			finishedStatus == EBLTTestStatus::EBLTTest_Failed ? TEXT("See output logs folder!") : TEXT("")),
			true);
	}

	// When testing, output incorrect test cases. When tuning, output correct test cases
	if (isTesting)
	{
		if (finishedStatus == EBLTTestStatus::EBLTTest_Failed)
		{
			testWhoFinished->OutputTestFailedCase();
		}
	}
	else
	{
		if (finishedStatus == EBLTTestStatus::EBLTTest_Success)
		{
			testWhoFinished->OutputTestSuccedTuningCase();
		}
	}

	// Remove the test actor from the set of running and destroy it
	m_currentlyRunningTests.Remove(testWhoFinished);

#if 0
	testWhoFinished->Destroy();
#endif

	if (UEBltBPLibrary::m_lastTestCharacterSpawned != nullptr && !UEBltBPLibrary::m_lastTestCharacterSpawned->IsPendingKillPending())
	{
		UEBltBPLibrary::m_lastTestCharacterSpawned->K2_DestroyActor();
	}
	UEBltBPLibrary::m_lastTestCharacterSpawned = nullptr;

	// If currently running tests is emtpy move back to idle
	if (m_currentlyRunningTests.Num() == 0)
	{
		const bool hasNextTest = internal_getNextTestToRun();

		if (!hasNextTest)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 1000, FColor::Emerald, FString::Printf(TEXT("============= FINISHED ALL TESTS =========== ")), true);

			if (m_continuousTestRunning)
			{
				m_state = EBLTManagerState::EBLT_Idle;
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 1000, FColor::Emerald, FString::Printf(TEXT("##### RESTARTING AS SPECIFIED in the options ##### ")), true);
			}
		}
	}

}


PRAGMA_ENABLE_OPTIMIZATION

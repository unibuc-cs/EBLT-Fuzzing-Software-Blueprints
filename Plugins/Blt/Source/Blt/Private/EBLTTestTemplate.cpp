// Fill out your copyright notice in the Description page of Project Settings.


#include "EBLTTestTemplate.h"
#include "EBltBPLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "EBLTManager.h"

#pragma optimize("", off)

// Sets default values
AEBLTTestTemplate::AEBLTTestTemplate()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEBLTTestTemplate::BeginPlay()
{
	Super::BeginPlay();

}

void AEBLTTestTemplate::Internal_SetupContext()
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEBLTManager::StaticClass(), OutActors);
	m_ebltManager = Cast<AEBLTManager>(OutActors[0]);

	// TODO: get the result of the function and check properly with manager 
	const bool res = SetupContext();
	UE_LOG(LogTemp, Warning, TEXT("Test run setup is: %d"), (int)res);
	if (!res)
	{
		//DestroyActor(this);
	}

	m_EBLTTestStatus = EBLTTestStatus::EBLTTest_NotRunning;
	OnContextSetupFinished();
}

// Called every frame
void AEBLTTestTemplate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	EBLTTestStatus prevTestStatus = m_EBLTTestStatus;


	// Step 0 : Check proper action against current state
	if (m_EBLTTestStatus == EBLTTestStatus::EBLTTest_NotSetup)
	{
		// Nothing to do
	}

	else if (m_EBLTTestStatus == EBLTTestStatus::EBLTTest_NotRunning)
	{
		CheckTriggers();
	}
	else if (m_EBLTTestStatus == EBLTTestStatus::EBLTTest_Failed)
	{
		 // TODO: destroy this actor !!! report test failed to manager
	}
	else if (m_EBLTTestStatus == EBLTTestStatus::EBLTTest_Success)
	{
		// TODO: destroy the actor, report this test as being successful
	}
	else if (m_EBLTTestStatus == EBLTTestStatus::EBLTTest_InProgress)
	{
		m_EBLTTestStatus = Internal_CheckTestCorrectness();
	}

	else
	{
		ensureMsgf(false, TEXT("Unknown test status %d !"), (int)m_EBLTTestStatus);
	}

	// Step 1 : Check proper action against state change 
	if (prevTestStatus != m_EBLTTestStatus)
	{
		// If finished, inform manager
		if (m_EBLTTestStatus == EBLTTestStatus::EBLTTest_Failed || m_EBLTTestStatus == EBLTTestStatus::EBLTTest_Success)
		{
			AEBLTTestTemplate::Internal_OnTestFinished();
		}
	}
}

void AEBLTTestTemplate::Internal_OnTestFinished()
{
	// Call corresponding blueprints events to let them know

	if (m_EBLTTestStatus == EBLTTestStatus::EBLTTest_Failed) 
	{
		OnTestFailed();
	}
	else
	{
		OnTestSucceeded();
	}

	m_ebltManager->OnTestFinished(this, m_EBLTTestStatus);
}


bool AEBLTTestTemplate::SetupContext_Implementation()
{
	// To implement in children
	return true;
}

bool AEBLTTestTemplate::CheckTriggers_Implementation()
{
	// To implement in children
	return true;
}

EBLTTestStatus AEBLTTestTemplate::CheckTestCorrectness_Implementation()
{
	return m_EBLTTestStatus;
}


EBLTTestStatus AEBLTTestTemplate::Internal_CheckTestCorrectness()
{
	// Step 1: Test foundation things that authoritive from the template test base class
	ensure(m_testAnnotations->m_spawnedTestActorForTest == this);

	for (auto& it : m_testAnnotations->m_OutputVarToAnnotationData)
	{
		const FString& varName = it.Key;
		const IGenericVarAnnotation* varAnnotation = it.Value;

		if (varAnnotation->m_outputCheckType == VariableCheckType::VARCHECK_FRAME_SAMPLE)
		{
			// TODO: check tick limits. Need to finish the implementation of allowing testing at different sample rates.
			// E.g. hold a frame counter..


			// Now read the content of the variable
			// TODO: implemented for other types too.
			if (varAnnotation->GetTestVariableType() == TestVariableType::TEST_VAR_FLOAT)
			{
				const VarAnnotation_Float* annotationAsFloat = (VarAnnotation_Float*)varAnnotation;
				const FDoubleProperty* propertyAsFloat = CastField<FDoubleProperty>(varAnnotation->m_parentUEPropertyRef);
				double* value = propertyAsFloat->GetPropertyValuePtr_InContainer(this);

				if (*value > annotationAsFloat->GetMaxVal())
				{
					m_EBLTTestStatus = EBLTTestStatus::EBLTTest_Failed;
				}
			}
		}
	}

	// Step 2: Check the test correctness too from the blueprint logic too if still the case
	if (m_EBLTTestStatus == EBLTTestStatus::EBLTTest_InProgress)
	{
		CheckTestCorrectness();
	}

	return m_EBLTTestStatus;
}



#pragma optimize("", on)

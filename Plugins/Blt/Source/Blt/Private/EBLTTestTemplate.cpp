// Fill out your copyright notice in the Description page of Project Settings.


#include "EBLTTestTemplate.h"
#include "EBltBPLibrary.h"

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

void AEBLTTestTemplate::InternalTestSetupContext_Implementation()
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
}

// Called every frame
void AEBLTTestTemplate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (m_EBLTTestStatus == EBLTTestStatus::EBLTTest_NotRunning)
		CheckTriggers();

	if (m_EBLTTestStatus == EBLTTestStatus::EBLTTest_Failed)
	{
		 // TODO: destroy this actor !!! report test failed to manager
	}
	else if (m_EBLTTestStatus == EBLTTestStatus::EBLTTest_InProgress)
	{
		m_EBLTTestStatus = CheckTestCorrectness();
	}
	else if (m_EBLTTestStatus == EBLTTestStatus::EBLTTest_Success)
	{
		 // TODO: destroy the actor, report this test as being successful
	}
	else
	{
		ensureMsgf(false, TEXT("Unknown test status %d !"), (int)m_EBLTTestStatus);
	}
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
	// To implement in children
	return m_EBLTTestStatus;
}


// Fill out your copyright notice in the Description page of Project Settings.


#include "EBLTManager.h"
#include "EngineUtils.h"
#include "Misc/OutputDeviceNull.h"

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
	
}

// Called every frame
void AEBLTManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);



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

		RunPathfindingTest(targetDestActor, targetTestActor);
	}
}


void AEBLTManager::OnMoveCompletedEvent_Implementation(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	int a = 3;
	a++;
}

#pragma optimize("", on)

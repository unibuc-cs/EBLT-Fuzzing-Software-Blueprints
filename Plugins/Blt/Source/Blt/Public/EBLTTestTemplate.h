// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EBltBPLibrary.h"
#include "TestAnnotation.h"
#include "EBLTTestTemplate.generated.h"

UCLASS()
class EBLT_API AEBLTTestTemplate : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEBLTTestTemplate();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="EBLT/TestsDefinition")
	bool SetupContext();

	bool SetupContext_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "EBLT/TestsDefinition")
	bool CheckTriggers();

	bool CheckTriggers_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "EBLT/TestsDefinition")
	EBLTTestStatus CheckTestCorrectness();

	EBLTTestStatus CheckTestCorrectness_Implementation();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "EBLT/TestsDefinition")
	void OnContextSetupFinished();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Internal_SetupContext();

	void SetTestAnnotations(SingleTestAnnotations* TestAnnotations) { m_testAnnotations = TestAnnotations; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TestInterns")
	AEBLTManager* m_ebltManager = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TestInterns")
	EBLTTestStatus m_EBLTTestStatus = EBLTTestStatus::EBLTTest_NotSetup;

private:
	SingleTestAnnotations* m_testAnnotations = nullptr;
	
};

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

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "EBLT/TestsDefinition")
	void OnTestFailed();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "EBLT/TestsDefinition")
	void OnTestSucceeded();


	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Internal_SetupContext();
	EBLTTestStatus Internal_CheckTestCorrectness();

	void Internal_OnTestFinished();

	void SetTestAnnotations(SingleTestAnnotations* TestAnnotations);

	// Set the string value of a variable
	void debugSetVarValue(const FString& varName, const FString& varValue);

	// Write the CSV for failure cases regarding this test
	// Create the file to write the output failure test cases if not already there
	// Each failed test will be on a row
	void OutputTestFailedCase();

	void OutputTestSuccedTuningCase();


	void SetGivenName(const FString& name) { m_givenName = name; }
	const FString& GetGivenName() const { return m_givenName; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	void OnTestFilureRecoveryElapsed();

public:	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TestInterns")
	AEBLTManager* m_ebltManager = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TestInterns")
	EBLTTestStatus m_EBLTTestStatus = EBLTTestStatus::EBLTTest_NotSetup;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TestInterns")
	EBLTType m_EBLTType = EBLTType::EBLT_FuzzForTesting;

private:
	SingleTestAnnotations* m_testAnnotations = nullptr;
	FString m_givenName;

	// This map contains the the mapping between all variables instances and their value, as string.
	// This can be used further for debugging purposes
	TMap<FString, FString> m_instanceVarNameToValueStr;
};

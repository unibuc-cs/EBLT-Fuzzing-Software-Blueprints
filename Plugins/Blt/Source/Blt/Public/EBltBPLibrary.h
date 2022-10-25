// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <Kismet/BlueprintFunctionLibrary.h>

#include "AIModule\Classes\Navigation\PathFollowingComponent.h"
#include "EBLTBPLibrary.generated.h"


class FJsonValue;
class AEBLTManager;

// Gets the UWorld for tests
static UWorld* GetWorldForTests()
{
	if (GEngine)
	{
		if (const FWorldContext* WorldContext = GEngine->GetWorldContextFromPIEInstance(0))
		{
			return WorldContext->World();
		}
	}
	return nullptr;
}



UCLASS(Abstract)
class UEBltBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//static bool ParseJson(const FString& FilePath, TSharedPtr<FJsonObject>& OutObject);


	//UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game Testing")
	//static bool GetAbsolutePath(const FString& FilePath, FString& AbsoluteFilePath);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game Testing")
	static UClass* FindClass(
		const FString& ClassName,
		const bool& bExactClass = false,
		UObject* const Package = nullptr
	);
	
	UFUNCTION(BlueprintCallable, Category = "EBLT/Utilities", meta = (WorldContext = "WorldContextObject"))
	static void GetAllActorsOfClass(const UObject* const WorldContextObject, const FString& ActorClassName, TArray<AActor*>& outActorsByClassName);


	// Gets the character to test in the world
	UFUNCTION(BlueprintCallable, Category = "EBLT/Utilities", meta = (WorldContext = "WorldContextObject"))
	static AActor* SpawnTestingCharacter(const UObject* const WorldContextObject, const FVector& location, const FRotator& rotation, const double characterScale);
	
	static void ApplyFuzzing(
		const UObject* const WorldContextObject,
		const FString& FilePath,
		const int32 Flags,
		const TArray<AActor*>& AffectedActors = TArray<AActor*>(),
		const bool bUseArray = false
	);
	
	UFUNCTION(BlueprintCallable, Category = "Game Testing", meta = (
		DisplayName = "Apply Fuzzing",
		WorldContext = "WorldContextObject",
		AutoCreateRefTerm = "AffectedActors"
	))
	static void K2ApplyFuzzing(
		const UObject* const WorldContextObject,
		const FString& FilePath,
		UPARAM(meta = (Bitmask, BitmaskEnum = "EFuzzingFlags")) const int32 Flags,
		const TArray<AActor*>& AffectedActors,
		const bool bUseArray = false
	);


	// Given a set of properties to be set from a JSON spec file, searches for all properties on the actor that are part of the spec file and randomizes them according to the rules
	static void RandomiseProperties(
		AActor* const& Actor,
		const UClass* const& JsonActorClassType,
		const TMap<FString, TSharedPtr<FJsonValue>>& ActorClassProperties,
		const int32 FuzzingFlags
	);

	static void RandomisePropertiesDefault(AActor* const& Actor, const FProperty* const& Property);
	
	static void RandomiseNumericProperty(
		AActor* const& Actor,
		const FProperty* const& Property,
		const FJsonValue* const& PropertyValue = nullptr
	);
	
	static void RandomiseStringProperty(
		AActor* const& Actor,
		const FProperty* const& Property,
		const FJsonValue* const& PropertyValue = nullptr
	);


protected:

	static void SolveEBLTManager(UWorld* world);
	static AEBLTManager* m_ebltManager;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#include "EBltBPLibrary.h"

#include "AIController.h"
#include "EBLTManager.h"
#include "EBLTTestTemplate.h"
#include "FuzzingFlags.h"
#include "Kismet/GameplayStatics.h"
#include "PythonBridge.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#include "TestAnnotation.h"
#include "AI/NavigationSystemBase.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"



PRAGMA_DISABLE_OPTIMIZATION

ACharacter* UEBltBPLibrary::m_lastTestCharacterSpawned = nullptr;


// Needed to exit from the current opened map.
static void ExitCurrentOpenedMap()
{
	if (UWorld* World = UEBltBPLibrary::GetWorldForTests())
	{
		if (APlayerController* TargetPC = UGameplayStatics::GetPlayerController(World, 0))
		{
			TargetPC->ConsoleCommand(TEXT("Exit"), true);
		}
	}
}

// Simulate a key pressed by name and input event (mouse / keyboard /ctrl)
static bool SimulatePressKey(const FName& KeyName, EInputEvent InputEvent)
{
	if (GEngine)
	{
		if (GEngine->GameViewport)
		{
			if (FViewport* Viewport = GEngine->GameViewport->Viewport)
			{
				if (FViewportClient* ViewportClient = Viewport->GetClient())
				{
					return ViewportClient->InputKey(FInputKeyEventArgs(Viewport, 0, KeyName, InputEvent));
				}
			}
		}
	}
	return false;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(Annotation_BaseTests, "EBLTOwnTests", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(Annotation_JsonTests, "EBLTOwnTests", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool Annotation_BaseTests::RunTest(FString const& Parameters)
{
	const auto Tolerance = KINDA_SMALL_NUMBER;

	VarAnnotation_Float test1;
	test1.Init("[min=0.8, max=1.2]");
	TestEqual("min val annotation ", test1.GetMinVal(), 0.8f);
	TestEqual("max val annotation ", test1.GetMaxVal(), 1.2f);


	VarAnnotation_Float test2;
	test2.Init("{100 # 200 # 300}");
	TestEqual("annotation type", test2.GetType(), VariableAnnotationType::VARANNOTATION_AS_SET);
	TestEqual("value on pos 2 ", test2.GetValues()[2], 300, Tolerance);


	VarAnnotation_Vector test3;
	test3.Init("{(-2330.000000,-1970.000000,543.147949) # (-1709.232910,-1860.210449,411.586578)}");
	TestEqual("annotation type", test3.GetType(), VariableAnnotationType::VARANNOTATION_AS_SET);
	TestEqual("second value z ", (float)test3.GetValues()[1].Z, 411.586578f, Tolerance);

	VarAnnotation_Vector test4;
	test4.Init("[min=(-2330.000000,-1970.000000,543.147949),max=(-1709.232910,-1860.210449,411.586578)]");
	TestEqual("annotation type", test4.GetType(), VariableAnnotationType::VARANNOTATIONS_AS_RANGE);
	TestEqual("max val annotation ", (float)test4.GetMaxVal().Y, -1860.210449f, Tolerance);

#if 0
	const auto BoolToTest = false;
	TestTrue("bool to test", BoolToTest);

	const auto FloatToTest = 0.F;
	const auto Expected = 10.F;
	
	TestEqual("float to test", FloatToTest, Expected, Tolerance);
#endif

	return true;
}

bool Annotation_JsonTests::RunTest(FString const& Parameters)
{
#if 0
	TSharedPtr<FJsonObject> JsonParsed;
	if (!UEBltBPLibrary::ParseJson("Data/AnnotationsExample.json", JsonParsed))
	{
		TestFalse("Can't find the json specified", false);
		return true;
	}
#endif
	/// TODO
	return true;
}

////////////////////////////////////////


AEBLTManager*  UEBltBPLibrary::m_ebltManager = nullptr;

UClass* UEBltBPLibrary::FindClass(const FString& ClassName, const bool& bExactClass, UObject* const Package)
{
	check(*ClassName);
	UObject* const Outer = Package;// ? Package : ANY_PACKAGE;

	if (UClass* const ClassType = FindFirstObjectSafe<UClass>(*ClassName))
		return ClassType;

	
	if (UClass* const ClassType = FindObject<UClass>(Outer, *ClassName, bExactClass))
		return ClassType;
	
	if (const UObjectRedirector* const RenamedClassRedirector
		= FindObject<UObjectRedirector>(Outer, *ClassName, bExactClass))
		return CastChecked<UClass>(RenamedClassRedirector->DestinationObject);
	
	UE_LOG(LogBlt, Warning, TEXT("Class %s could not be found!"), *ClassName);
	return nullptr;
}

void UEBltBPLibrary::GetAllActorsOfClass(
	const UObject* const WorldContextObject,
	const FString& ActorClassName,
	TArray<AActor*>& outActorsByClassName
)
{
	outActorsByClassName.Empty();


	const TSubclassOf<AActor> ActorClass = FindClass(ActorClassName);
	if (!ActorClass)
	{
		return;
	}

	UGameplayStatics::GetAllActorsOfClass(WorldContextObject->GetWorld(), ActorClass, outActorsByClassName);
}

void UEBltBPLibrary::ApplyFuzzing(
	const UObject* const WorldContextObject,
	const FString& FilePath,
	const int32 Flags,
	const TArray<AActor*>& AffectedActors,
	const bool bUseArray
)
{
#if 0
	TSharedPtr<FJsonObject> JsonParsed;
	if (!ParseJson(FilePath, JsonParsed))
		return;

	const TMap<FString, TSharedPtr<FJsonValue>> JsonClasses = JsonParsed.Get()->Values;
	for (const TTuple<FString, TSharedPtr<FJsonValue>>& JsonClass : JsonClasses)
	{
		const FString& ActorClassName = JsonClass.Key;
		const UClass* const& JsonActorClassType = FindClass(ActorClassName);
		if (!JsonActorClassType)
			continue;
		
		const TSharedPtr<FJsonObject>* ActorClassObject;
		if (!JsonClass.Value->TryGetObject(ActorClassObject))
		{
			UE_LOG(LogBlt, Error, TEXT("Entry %s must have an Object type value!"), *ActorClassName);
			continue;
		}

		TArray<AActor*> allActorsByClass;
		GetAllActorsOfClass(WorldContextObject, ActorClassName, allActorsByClass);

		const TMap<FString, Annotation>& ActorClassProperties = ActorClassObject->Get()->Values;
		for (AActor* const& Actor :
			bUseArray ? AffectedActors : allActorsByClass)
		{
			RandomiseProperties(Actor, JsonActorClassType, ActorClassProperties, Flags);
		}
	}
#endif
}

void UEBltBPLibrary::K2ApplyFuzzing(
	const UObject* const WorldContextObject,
	const FString& FilePath,
	const int32 Flags,
	const TArray<AActor*>& AffectedActors,
	const bool bUseArray
)
{
	ApplyFuzzing(WorldContextObject, FilePath, Flags, AffectedActors, bUseArray);
}

void UEBltBPLibrary:: RandomiseProperties(
	AActor* const& Actor,
	const UClass* const& JsonActorClassType,
	const TMap<FString, TSharedPtr<FJsonValue>>& ActorClassProperties,
	const int32 FuzzingFlags
)
{
	if (!Actor)
		return;
	
	const UClass* const& ActorClass = Actor->GetClass();
	if (!ActorClass->IsChildOf(JsonActorClassType))
		return;

	const bool& bIncludeBase = FuzzingFlags & static_cast<uint8>(EFuzzingFlags::IncludeBase);
	const bool& bIncludeSuper = FuzzingFlags & static_cast<uint8>(EFuzzingFlags::IncludeSuper);
	const bool& bIncludeNull = FuzzingFlags & static_cast<uint8>(EFuzzingFlags::IncludeNull);
	
 	for (TFieldIterator<FProperty> Iterator(ActorClass); Iterator; ++Iterator)
	{
		const FProperty* const Property = *Iterator;
		const FString& PropertyName = Property->GetNameCPP();
		
		if (!ActorClassProperties.Contains(PropertyName))
		{
			const UClass* const& OwnerClass = Property->GetOwnerClass();
			if (
				bIncludeBase && OwnerClass == JsonActorClassType ||
				bIncludeSuper && OwnerClass == JsonActorClassType->GetSuperClass()
			) {
				RandomisePropertiesDefault(Actor, Property);
			}
			continue;
		}

		const FJsonValue* const PropertyValue = ActorClassProperties.Find(PropertyName)->Get();
		switch (PropertyValue->Type)
		{
		case EJson::Array:
			RandomiseNumericProperty(Actor, Property, PropertyValue);
			break;

		case EJson::String:
			RandomiseStringProperty(Actor, Property, PropertyValue);
			break;

		case EJson::Null:
			if (bIncludeNull)
			{
				RandomisePropertiesDefault(Actor, Property);
			}
			break;
			
		default:
			break;
		}
	}
}

void UEBltBPLibrary::RandomisePropertiesDefault(AActor* const& Actor, const FProperty* const& Property)
{
	RandomiseNumericProperty(Actor, Property);
	RandomiseStringProperty(Actor, Property);
}

void UEBltBPLibrary::RandomiseNumericProperty(
	AActor* const& Actor,
	const FProperty* const& Property,
	const FJsonValue* const& PropertyValue
)
{
	const TArray<TSharedPtr<FJsonValue>>& Interval = PropertyValue ?
		PropertyValue->AsArray() : TArray<TSharedPtr<FJsonValue>>();
	
	const float& RandomNumber = FMath::FRandRange(
	Interval.Num() > 0u ?
		Interval[0u].Get()->AsNumber() : 0.0f,
		
	Interval.Num() > 1u ?
		Interval[1u].Get()->AsNumber() : FMath::FRandRange(0.0f, static_cast<float>(RAND_MAX))
	);
	
	if (const FNumericProperty* const NumericProperty = CastField<const FNumericProperty>(Property))
	{
		NumericProperty->SetNumericPropertyValueFromString(
			NumericProperty->ContainerPtrToValuePtr<float>(Actor),
			*FString::Printf(TEXT("%f"), RandomNumber)
		);
	}
	else if (const FBoolProperty* const BoolProperty = CastField<const FBoolProperty>(Property))
	{
		BoolProperty->SetPropertyValue_InContainer(Actor, static_cast<uint8>(RandomNumber) % 2u);
	}
	else if (PropertyValue)
	{
		UE_LOG(LogBlt, Error, TEXT("%s is not Numeric!"), *Property->GetFullName());
	}
}

void UEBltBPLibrary::RandomiseStringProperty(
	AActor* const& Actor,
	const FProperty* const& Property,
	const FJsonValue* const& PropertyValue
)
{
	const UPythonBridge* const PythonBridge = UPythonBridge::Get();
	if (!PythonBridge)
	{
		UE_LOG(LogBlt, Error, TEXT("Python bridge could not be instantiated!"));
		return;
	}
	
	const FString& RandomString = PythonBridge->GenerateStringFromRegex(
		PropertyValue ? PropertyValue->AsString() : "[\\w\\W\\s\\S\\d\\D]{:255}"
	);
	
	if (const FStrProperty* const StringProperty = CastField<const FStrProperty>(Property))
	{
		StringProperty->SetPropertyValue_InContainer(Actor, RandomString);
	}
	else if (const FNameProperty* const NameProperty = CastField<const FNameProperty>(Property))
	{
		NameProperty->SetPropertyValue_InContainer(Actor, FName(RandomString));
	}
	else if (const FTextProperty* const TextProperty = CastField<const FTextProperty>(Property))
	{
		TextProperty->SetPropertyValue_InContainer(Actor, FText::FromString(RandomString));
	}
	else if (PropertyValue)
	{
		UE_LOG(LogBlt, Error, TEXT("%s is not FString, FName or FText!"), *Property->GetFullName());
	}
}

void UEBltBPLibrary::SolveEBLTManager(UWorld* world)
{
	if (m_ebltManager)
		return;

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(world, AEBLTManager::StaticClass(), OutActors);
	ensureMsgf(OutActors.Num() == 1, TEXT("There should be a single ebltManager instanced"));
	m_ebltManager = Cast<AEBLTManager>(OutActors[0]);
}


AActor* UEBltBPLibrary::SpawnTestingCharacter(const UObject* const WorldContextObject, const FVector& location, const FRotator& rotation, const double characterScale, const double walkSpeed, const double sprintSpeed, const double jumpVelocity)
{
	ensureMsgf(m_lastTestCharacterSpawned == nullptr, TEXT("A previous character remained spawned !!!"));

	UWorld* world = WorldContextObject->GetWorld();
	SolveEBLTManager(world);

	const TSubclassOf<AActor> actorClass = FindClass("NPC_EBLT_TestCharacter.NPC_EBLT_TestCharacter_C");

	FTransform actorTransform(rotation, location, UE::Math::TVector(characterScale, characterScale, characterScale));
	actorTransform.SetScale3D(UE::Math::TVector(characterScale, characterScale, characterScale));

	// Modify a bit the location of the character to be on the navmesh
	FNavLocation preferedModLoc[3];
	bool prefferedModLoc_isgood[3];
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(world);
	prefferedModLoc_isgood[0] = NavSys->ProjectPointToNavigation(location, preferedModLoc[0], FVector(0.0, 0.0, 500.0f));
	prefferedModLoc_isgood[1] = NavSys->GetRandomPointInNavigableRadius(location, 100.0f, preferedModLoc[1]);
	prefferedModLoc_isgood[2] = NavSys->GetRandomReachablePointInRadius(location, 100.0f, preferedModLoc[2]);

	for (int i = 0; i < 3; i++)
	{
		if (!prefferedModLoc_isgood[i])
			continue;

		actorTransform.SetLocation(preferedModLoc[i]);
		break;
	}


	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ACharacter* actorSpawned = Cast<ACharacter>(world->SpawnActor(actorClass, &actorTransform, SpawnParameters));
	if (actorSpawned)
	{
		actorSpawned->SpawnDefaultController();


		UGameplayStatics::GetPlayerController(WorldContextObject, 0)->SetViewTarget(actorSpawned);


		AAIController* aiController = Cast<AAIController>(actorSpawned->GetController());
		aiController->ReceiveMoveCompleted.AddDynamic(m_ebltManager, &AEBLTManager::OnMoveCompletedEvent);
	}

	UCharacterMovementComponent* movementComponent = Cast<UCharacterMovementComponent>(actorSpawned->GetMovementComponent());
	movementComponent->MaxWalkSpeed = sprintSpeed;
	movementComponent->MaxFlySpeed = jumpVelocity;


	m_lastTestCharacterSpawned = actorSpawned;
	return actorSpawned;
}

void UEBltBPLibrary::OnEBLTManagerDestroyed()
{
	m_lastTestCharacterSpawned = nullptr;
	m_ebltManager = nullptr;
}

UWorld* UEBltBPLibrary::GetWorldForTests()
{
	if (GEngine)
	{
		if (const FWorldContext* WorldContext = GEngine->GetWorldContextFromPIEInstance(0))
		{
			return WorldContext->World();
		}
		if (GEngine->GetWorld())
		{
			return GEngine->GetWorld();
		}
		else
		{
			return AEBLTManager::getMyWorld();
		}
	}
	return nullptr;
}


PRAGMA_ENABLE_OPTIMIZATION


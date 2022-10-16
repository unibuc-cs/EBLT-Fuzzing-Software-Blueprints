// Copyright Epic Games, Inc. All Rights Reserved.

#include "EBltBPLibrary.h"

#include "AIController.h"
#include "EBLTManager.h"
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


#pragma optimize("", off)

///////////// DEMO DELETE 
IMPLEMENT_SIMPLE_AUTOMATION_TEST(AnnotationTest_IntSet, "EBLTOwnTests", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool AnnotationTest_IntSet::RunTest(FString const& Parameters)
{
	TestAnnotation_Float test1("[min=0.8, max=1.2]");


	const auto Tolerance = KINDA_SMALL_NUMBER;
	TestEqual("min val annotation ", test1.GetMinVal(), 0.8f);
	TestEqual("max val annotation ", test1.GetMaxVal(), 1.2f);

	TestAnnotation_Float test2("{100 # 200 # 300}");

	
	TestEqual("annotation type", test1.GetType(), VariableAnnotationType::VARANNOTATION_AS_SET);
	TestEqual("max val annotation ", test1.GetValues()[2], 300, Tolerance);

#if 0
	const auto BoolToTest = false;
	TestTrue("bool to test", BoolToTest);

	const auto FloatToTest = 0.F;
	const auto Expected = 10.F;
	
	TestEqual("float to test", FloatToTest, Expected, Tolerance);
#endif

	return true;
}

////////////////////////////////////////



DEFINE_LOG_CATEGORY(LogBlt);


AEBLTManager*  UEBltBPLibrary::m_ebltManager = nullptr;


bool UEBltBPLibrary::ParseJson(const FString& FilePath, TSharedPtr<FJsonObject>& OutObject)
{
	FString AbsoluteFilePath;
	if (!GetAbsolutePath(FilePath, AbsoluteFilePath))
		return false;
	
	FString JsonRaw;
	FFileHelper::LoadFileToString(JsonRaw, *AbsoluteFilePath);
	if (!FJsonSerializer::Deserialize<TCHAR>(TJsonReaderFactory<TCHAR>::Create(JsonRaw), OutObject))
	{
		UE_LOG(LogBlt, Error, TEXT("Could not deserialize %s [check if file is JSON]"), *AbsoluteFilePath);
		return false;
	}

	return true;
}

bool UEBltBPLibrary::GetAbsolutePath(const FString& FilePath, FString& AbsoluteFilePath)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	
	AbsoluteFilePath = FilePath;
	if (PlatformFile.FileExists(*AbsoluteFilePath))
		return true;

	AbsoluteFilePath = FPaths::ProjectContentDir() + FilePath;
	if (PlatformFile.FileExists(*AbsoluteFilePath))
		return true;

	UE_LOG(LogBlt, Error, TEXT("File %s not found [relative path starts from /Content/]"), *AbsoluteFilePath);
	return false;
}

UClass* UEBltBPLibrary::FindClass(const FString& ClassName, const bool& bExactClass, UObject* const Package)
{
	check(*ClassName);
	UObject* const Outer = Package ? Package : ANY_PACKAGE;
	
	if (UClass* const ClassType = FindObject<UClass>(Outer, *ClassName, bExactClass))
		return ClassType;

	if (const UObjectRedirector* const RenamedClassRedirector
		= FindObject<UObjectRedirector>(Outer, *ClassName, bExactClass))
		return CastChecked<UClass>(RenamedClassRedirector->DestinationObject);

	UE_LOG(LogBlt, Warning, TEXT("Class %s could not be found!"), *ClassName);
	return nullptr;
}

TArray<AActor*> UEBltBPLibrary::GetAllActorsOfClass(
	const UObject* const WorldContextObject,
	const FString& ActorClassName
)
{
	const TSubclassOf<AActor> ActorClass = FindClass(ActorClassName);
	if (!ActorClass)
		return TArray<AActor*>();

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject->GetWorld(), ActorClass, OutActors);
	return OutActors;
}

void UEBltBPLibrary::ApplyFuzzing(
	const UObject* const WorldContextObject,
	const FString& FilePath,
	const int32 Flags,
	const TArray<AActor*>& AffectedActors,
	const bool bUseArray
)
{
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

		const TMap<FString, TSharedPtr<FJsonValue>>& ActorClassProperties = ActorClassObject->Get()->Values;
		for (AActor* const& Actor :
			bUseArray ? AffectedActors : GetAllActorsOfClass(WorldContextObject, ActorClassName))
		{
			RandomiseProperties(Actor, JsonActorClassType, ActorClassProperties, Flags);
		}
	}
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


AActor* UEBltBPLibrary::SpawnTestingCharacter(const UObject* const WorldContextObject, const FVector& location, const FRotator& rotation, const double characterScale)
{
	UWorld* world = WorldContextObject->GetWorld();
	SolveEBLTManager(world);

	const TSubclassOf<AActor> actorClass = FindClass("NPC_EpicCharacter.NPC_EpicCharacter_C");

	FTransform actorTransform(rotation, location, UE::Math::TVector(characterScale, characterScale, characterScale));
	actorTransform.SetScale3D(UE::Math::TVector(characterScale, characterScale, characterScale));

	ACharacter* actorSpawned = Cast<ACharacter>(world->SpawnActor(actorClass, &actorTransform));
	actorSpawned->SpawnDefaultController();

	Cast<AAIController>(actorSpawned->GetController())->ReceiveMoveCompleted.AddDynamic(m_ebltManager, &AEBLTManager::OnMoveCompletedEvent);
	return actorSpawned;
}


#pragma optimize("", on)

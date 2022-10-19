#include "TestAnnotation.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Logging/LogMacros.h"
#include <algorithm>

#include "EBltBPLibrary.h"

#include "Containers/StringConv.h"
#include "Kismet/GameplayStatics.h"
#pragma optimize("", off)


DEFINE_LOG_CATEGORY(LogBlt);

bool TestAnnotation_Float::Init(const std::string& genericAnnotation)
{
	m_testVariableType = TestVariableType::TEST_VAR_FLOAT;
	return readAllValues(genericAnnotation);
}

float TestAnnotation_Float::generateRandomValue() const
{
	if (m_eAnnotationType == VariableAnnotationType::VARANNOTATION_AS_SET)
	{
		return m_setOfValues[rand() % m_setOfValues.size()];
	}
	else
	{
		float res = ((float)rand() / (RAND_MAX));
		res = m_minVal + (m_maxVal - m_minVal) * res;

		return res;
	}
}

float TestAnnotation_Float::readSingleValue(std::string& singleValueStr)
{
	ITestAnnotation::readSingleValue(singleValueStr);

	float res = 0.0f;
	const int numRead = sscanf_s(singleValueStr.c_str(), "%f", &res);
	m_isValid = m_isValid && (numRead == 1);
	return res;
}

bool TestAnnotation_Int::Init(const std::string& genericAnnotation)
{
	m_testVariableType = TestVariableType::TEST_VAR_INTEGER;

	return readAllValues(genericAnnotation);
}

int TestAnnotation_Int::generateRandomValue() const
{
	if (m_eAnnotationType == VariableAnnotationType::VARANNOTATION_AS_SET)
	{
		return m_setOfValues[rand() % m_setOfValues.size()];
	}
	else
	{
		float res = ((float)rand() / (RAND_MAX));
		res = m_minVal + (m_maxVal - m_minVal) * res;

		return (int)res;
	}
}

int TestAnnotation_Int::readSingleValue(std::string& singleValueStr)
{
	ITestAnnotation::readSingleValue(singleValueStr);

	int res = 0;
	const int numRead = sscanf_s(singleValueStr.c_str(), "%d", &res);
	m_isValid = m_isValid && (numRead == 1);
	return res;
}

bool TestAnnotation_Vector::Init(const std::string& genericAnnotation)
{
	m_testVariableType = TestVariableType::TEST_VAR_VECTOR;

	return readAllValues(genericAnnotation);
}

FVector3f TestAnnotation_Vector::generateRandomValue() const
{
	if (m_eAnnotationType == VariableAnnotationType::VARANNOTATION_AS_SET)
	{
		return m_setOfValues[rand() % m_setOfValues.size()];
	}
	else
	{
		const float interpFactor = ((float)rand() / (RAND_MAX));
		const FVector3f res = FMath::Lerp(m_minVal, m_maxVal, res);

		return res;
	}
}

FVector3f TestAnnotation_Vector::readSingleValue(std::string& singleValueStr)
{
	ITestAnnotation::readSingleValue(singleValueStr);

	FVector3f res;
	const int numCharsRed = sscanf_s(singleValueStr.c_str(), "(%f,%f,%f)", &res.X, &res.Y, &res.Z);
	m_isValid = m_isValid && (numCharsRed == 3);

	return res;
}


class VariableAnnotationFactory
{
public:
	static IGenericTestAnnotation* CreateFromJsonValue(const FJsonValue* const jsonVarSpec, const FProperty* const targetProperty)
	{
		IGenericTestAnnotation* res = nullptr;

		// TODO: need more work to add support from the other pieces of code here for reusability
		ensureMsgf(jsonVarSpec->Type == EJson::String, TEXT("This function supports for now just these types"));
		const std::string jsonSpecStr = std::string(TCHAR_TO_UTF8(*jsonVarSpec->AsString()));

		// Is Float / Int ?
		if (CastChecked<FNumericProperty>(targetProperty))
		{
			if (CastChecked<FFloatProperty>(targetProperty))
			{
				res = NewObject < TestAnnotation_Float>();
				res->Init(jsonSpecStr);
			}
			else if (CastChecked<FIntProperty>(targetProperty))
			{
				res = NewObject<TestAnnotation_Int>();
				res->Init(jsonSpecStr);
			}
			else
			{
				ensureMsgf(false, TEXT("not yet"));
			}
		}
		else if (CastChecked<FStructProperty>(targetProperty))// Is Vector3D ?
		{
			res = NewObject<TestAnnotation_Vector>();
			res->Init(jsonSpecStr);
		}

		ensureMsgf(res->IsValid(), TEXT("Your data for format wasn't valid"));


		res->m_parentUEPropertyRef = targetProperty;

		return res;
	}
};

bool TestsAnnotationsHelper::ParseTestsAnnotationsFromJSon(const FString& FilePath, MapFromTestNameToAnnotations& outTestsAndAnnotations)
{
	FString AbsoluteFilePath;
	if (!EBLTCommonUtils::GetAbsolutePath(FilePath, AbsoluteFilePath))
		return false;

	FString JsonRaw;
	FFileHelper::LoadFileToString(JsonRaw, *AbsoluteFilePath);
	TSharedPtr<FJsonObject> JsonParsed;
	if (!FJsonSerializer::Deserialize<TCHAR>(TJsonReaderFactory<TCHAR>::Create(JsonRaw), JsonParsed))
	{
		UE_LOG(LogBlt, Error, TEXT("Could not deserialize %s [check if file is JSON]"), *AbsoluteFilePath);
		return false;
	}

	const TMap<FString, TSharedPtr<FJsonValue>> JsonClasses = JsonParsed.Get()->Values;
	for (const TTuple<FString, TSharedPtr<FJsonValue>>& BlueprintToTestClassDef : JsonClasses)
	{
		const FString& ActorClassName = BlueprintToTestClassDef.Key;
		const UClass* const BlueprintToTestClassType = UEBltBPLibrary::FindClass(ActorClassName);
		if (!BlueprintToTestClassType)
			continue;

		SingleTestAnnotations testDef;
		testDef.m_classToTest = BlueprintToTestClassType;

		const TSharedPtr<FJsonObject>* BlueprintToTestSpec;
		if (!BlueprintToTestClassDef.Value->TryGetObject(BlueprintToTestSpec))
		{
			UE_LOG(LogBlt, Error, TEXT("Entry %s must have an Object type value!"), *ActorClassName);
			continue;
		}

		const TMap<FString, TSharedPtr<FJsonValue>>& BlueprintToTestPropertiesSpec = BlueprintToTestSpec->Get()->Values;

		for (TFieldIterator<FProperty> Iterator(BlueprintToTestClassType); Iterator; ++Iterator)
		{
			const FProperty* const Property = *Iterator;
			const FString& PropertyName = Property->GetNameCPP();

			if (!BlueprintToTestPropertiesSpec.Contains(PropertyName))
			{
				continue;
			}

			const FJsonValue* const PropertyValue = BlueprintToTestPropertiesSpec.Find(PropertyName)->Get();

			IGenericTestAnnotation* varAnnotationData = VariableAnnotationFactory::CreateFromJsonValue(PropertyValue, Property);
			if (varAnnotationData == nullptr)
			{
				ensureMsgf(false, TEXT("Invalid data"));
				return false;
			}

			testDef.m_VariableNameToAnnotationData.Add(PropertyName, varAnnotationData);
		}
	}

#if 0
		TArray<AActor*> outActorsByClassName;
		UEBltBPLibrary::GetAllActorsOfClass(GetWorldForTests(), ActorClassName, outActorsByClassName);

		for (AActor* const actorTarget : outActorsByClassName)
		{
			UEBltBPLibrary::RandomiseProperties(actorTarget, JsonActorClassType, ActorClassProperties, (int32)EFuzzingFlags::All);
		}

#endif

	return true;
}

bool TestsAnnotationsHelper::BuildTestInstance(const UWorld* worldContext,
												const TestParamsSuggestionStrategy strategy, 
												AActor* targetTestActor, 
												const SingleTestAnnotations& testAnnotations)
{
	switch(strategy)
	{
	case TestParamsSuggestionStrategy::TESTPARAMSTRATEGY_RANDOM:
		{
			// TODO: here we should select which actors...maybe there are more that we can do !
			TArray<AActor*> outAllActors;
			UGameplayStatics::GetAllActorsOfClass(worldContext, testAnnotations.m_classToTest, outAllActors);


			// For each actor, just randomize properties according to the annotations
			for (const AActor* actor : outAllActors)
			{
				for (const TPair<FString, IGenericTestAnnotation>& varSpec : testAnnotations.m_VariableNameToAnnotationData)
				{
					IGenericTestAnnotation* varAnnotation = varSpec.Value;

					
					switch (varAnnotation->GetTestVariableType())
					{
						case TestVariableType::TEST_VAR_VECTOR:
						{
							const TestAnnotation_Vector* asVector = CastChecked<TestAnnotation_Vector>(varAnnotation);
							const FVector3f val = asVector->generateRandomValue();
						}
						break;

						case TestVariableType::TEST_VAR_FLOAT:
						{
							const TestAnnotation_Float* asFloat = CastChecked<TestAnnotation_Float>(varAnnotation);
							const float val = asFloat->generateRandomValue();
						}
						break;

						case TestVariableType::TEST_VAR_INTEGER:
						{
							const TestAnnotation_Int* asInt= CastChecked<TestAnnotation_Float>(varAnnotation);
							const int val = asInt->generateRandomValue();
						}
						break;

						default:
						{
							ensureMsgf(false, TEXT("Not knwon %d"), int(varAnnotation->GetTestVariableType()));
						}
						break;
					}
					
				}
			}
		}
		break;
	case TestParamsSuggestionStrategy::TESTPARAMSSTRATEGY_RL:
		{
		ensureMsgf(false, TEXT("Next submission"));
		}
	}
}


#pragma optimize("", on)

#include "TestAnnotation.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Logging/LogMacros.h"
#include <algorithm>
#include <sstream>

#include "EBltBPLibrary.h"
#include "EBLTTestTemplate.h"
#include "EngineUtils.h"
#include "csv.hpp"

#include "Containers/StringConv.h"
#include "Kismet/GameplayStatics.h"
#pragma optimize("", off)


DEFINE_LOG_CATEGORY(LogBlt);

bool VarAnnotation_Float::Init(const std::string& genericAnnotation)
{
	m_testVariableType = TestVariableType::TEST_VAR_FLOAT;
	return readAllValues(genericAnnotation);
}

float VarAnnotation_Float::generateRandomValue() const
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

float VarAnnotation_Float::readSingleValue(std::string& singleValueStr)
{
	IVarAnnotation::readSingleValue(singleValueStr);

	float res = 0.0f;
	const int numRead = sscanf_s(singleValueStr.c_str(), "%f", &res);
	m_isValid = m_isValid && (numRead == 1);
	return res;
}

bool VarAnnotation_Int::Init(const std::string& genericAnnotation)
{
	m_testVariableType = TestVariableType::TEST_VAR_INTEGER;

	return readAllValues(genericAnnotation);
}

int VarAnnotation_Int::generateRandomValue() const
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

int VarAnnotation_Int::readSingleValue(std::string& singleValueStr)
{
	IVarAnnotation::readSingleValue(singleValueStr);

	int res = 0;
	const int numRead = sscanf_s(singleValueStr.c_str(), "%d", &res);
	m_isValid = m_isValid && (numRead == 1);
	return res;
}

bool VarAnnotation_Vector::Init(const std::string& genericAnnotation)
{
	m_testVariableType = TestVariableType::TEST_VAR_VECTOR;

	return readAllValues(genericAnnotation);
}

FVector3f VarAnnotation_Vector::generateRandomValue() const
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

FVector3f VarAnnotation_Vector::readSingleValue(std::string& singleValueStr)
{
	IVarAnnotation::readSingleValue(singleValueStr);

	FVector3f res;
	const int numCharsRed = sscanf_s(singleValueStr.c_str(), "(%f,%f,%f)", &res.X, &res.Y, &res.Z);
	m_isValid = m_isValid && (numCharsRed == 3);

	return res;
}

bool VarAnnotation_Entity::Init(const std::string& genericAnnotation)
{
	m_testVariableType = TestVariableType::TEST_VAR_OBJECT;

	return readAllValues(genericAnnotation);
}

AActor* VarAnnotation_Entity::generateRandomValue() const
{
	if (m_eAnnotationType == VariableAnnotationType::VARANNOTATION_AS_SET)
	{
		return m_setOfValues[rand() % m_setOfValues.size()];
	}
	else
	{
		ensure(false);
		return nullptr;
	}
}

AActor* VarAnnotation_Entity::readSingleValue(std::string& singleValueStr)
{
	UWorld* world = GetWorldForTests();
	IVarAnnotation::readSingleValue(singleValueStr);

	//char buff_out[256];
	//const int numRead = sscanf_s(singleValueStr.c_str(), "%s", buff_out);
	//m_isValid = m_isValid && (numRead == 1);

	// COuld use tags to be much easier - for instance tag all test entitiies with something
	const FString entityNameAsString = FString(singleValueStr.c_str());

	TArray<AActor*> outAllActors;
	UGameplayStatics::GetAllActorsOfClassWithTag(world, AActor::StaticClass(), FName( "EBLT"), outAllActors);

	AActor* foundActor = nullptr;
	for (AActor* actor : outAllActors)
	{
		
		if (actor->GetActorNameOrLabel().Contains(entityNameAsString))
		{
			foundActor = actor;
			break;
		}
	}

	m_isValid = m_isValid && (foundActor != nullptr);

	ensureMsgf(foundActor, TEXT("Could not find actor named %s"), *entityNameAsString);
	return foundActor;
}



class VariableAnnotationFactory
{
public:
	static IGenericVarAnnotation* CreateFromJsonValue(const FJsonValue* const jsonVarSpec, const FProperty* const targetProperty, bool isInputVar)
	{
		IGenericVarAnnotation* res = nullptr;

		// TODO: need more work to add support from the other pieces of code here for reusability
		std::string jsonValueSpecStr, jsonValueSpecType, failCodeStr, succeedCodeStr;

		if (isInputVar)
		{
			ensureMsgf(jsonVarSpec->Type == EJson::String, TEXT("This function supports for now just these types"));
			jsonValueSpecStr = std::string(TCHAR_TO_UTF8(*jsonVarSpec->AsString()));
		}
		else
		{
			ensureMsgf(jsonVarSpec->Type == EJson::Object, TEXT("This function supports for now just these types"));
			ensureMsgf(jsonVarSpec->AsObject()->Values.Contains("type") && jsonVarSpec->AsObject()->Values.Contains("value"), TEXT("The object does not contains type and value keys"));

			ensureMsgf(jsonVarSpec->AsObject()->Values.Contains("failCode") ^ jsonVarSpec->AsObject()->Values.Contains("succeedCode"), TEXT("The var def doesn't contain one of the codes"));

			jsonValueSpecType = std::string(TCHAR_TO_UTF8(*jsonVarSpec->AsObject()->Values.Find("type")->Get()->AsString()));
			jsonValueSpecStr = std::string(TCHAR_TO_UTF8( *jsonVarSpec->AsObject()->Values.Find("value")->Get()->AsString()));

			if (jsonVarSpec->AsObject()->Values.Contains("failCode"))
			{
				failCodeStr = std::string(TCHAR_TO_UTF8(*jsonVarSpec->AsObject()->Values.Find("failCode")->Get()->AsString()));
			}

			if (jsonVarSpec->AsObject()->Values.Contains("succeedCode"))
			{
				succeedCodeStr = std::string(TCHAR_TO_UTF8(*jsonVarSpec->AsObject()->Values.Find("succeedCode")->Get()->AsString()));
			}
		}

		// Is Float / Int ?
		if (CastField<FNumericProperty>(targetProperty))
		{
			if (CastField<FFloatProperty>(targetProperty) || CastField<FDoubleProperty>(targetProperty))
			{
				res = new  VarAnnotation_Float;
				res->Init(jsonValueSpecStr);
			}
			else if (CastField<FIntProperty>(targetProperty))
			{
				res = new VarAnnotation_Int;
				res->Init(jsonValueSpecStr);
			}
			else
			{
				ensureMsgf(false, TEXT("not yet"));
			}
		}
		else if (CastField<FStructProperty>(targetProperty))// Is Vector3D ?
		{
			res = new VarAnnotation_Vector;
			res->Init(jsonValueSpecStr);
		}
		else if (CastField<FObjectProperty>(targetProperty))
		{
			
			res = new VarAnnotation_Entity;
			res->Init(jsonValueSpecStr);
		}

		ensureMsgf(res->IsValid(), TEXT("Your data for format wasn't valid"));


		res->m_parentUEPropertyRef = targetProperty;


		// Process the output var specs
		int sampleRate = -1;
		VariableCheckType checkType = VariableCheckType::VARCHECK_AT_END_ONLY;
		if (!isInputVar)
		{
			if (jsonValueSpecType == "end")
			{
				checkType = VariableCheckType::VARCHECK_AT_END_ONLY;
			}
			else
			{
				const int numRead = sscanf_s(jsonValueSpecType.c_str(), "continuous-%d", &sampleRate);
				ensure(numRead == 1);
				checkType = VariableCheckType::VARCHECK_FRAME_SAMPLE;
			}


			int succeedCode = IGenericVarAnnotation::VAR_ANNOTATION_INVALID_RESULT, failCode = IGenericVarAnnotation::VAR_ANNOTATION_INVALID_RESULT;
			if (!succeedCodeStr.empty())
			{
				succeedCode = std::stoi(succeedCodeStr);
			}
			if (!failCodeStr.empty())
			{
				failCode = std::stoi(failCodeStr);
			}

			res->setOutputType(checkType, sampleRate, failCode, succeedCode);
		}

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
		UClass* const BlueprintToTestClassType = UEBltBPLibrary::FindClass(ActorClassName);
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

		const FString keys[2] = { "inputs", "expectedOutputs" };
		for (int keyIndex = 0; keyIndex < 2; keyIndex++)
		{
			const bool isInputVar = keyIndex == 0;
			if (!BlueprintToTestSpec->Get()->Values.Contains(keys[keyIndex]))
				continue;


			const TSharedPtr<FJsonObject>* BlueprintToTestPropertiesSpec;
			BlueprintToTestSpec->Get()->Values.Find(keys[keyIndex])->Get()->TryGetObject(BlueprintToTestPropertiesSpec);


			for (TFieldIterator<FProperty> Iterator(BlueprintToTestClassType); Iterator; ++Iterator)
			{
				const FProperty* const Property = *Iterator;
				const FString& PropertyName = Property->GetNameCPP();

				if (!BlueprintToTestPropertiesSpec->Get()->Values.Contains(PropertyName))
				{
					continue;
				}

				const FJsonValue* const PropertyValue = BlueprintToTestPropertiesSpec->Get()->Values.Find(PropertyName)->Get();

				IGenericVarAnnotation* varAnnotationData = VariableAnnotationFactory::CreateFromJsonValue(PropertyValue, Property, isInputVar);
				if (varAnnotationData == nullptr)
				{
					ensureMsgf(false, TEXT("Invalid data"));
					return false;
				}

				// Add either as input or output
				if (isInputVar)
				{
					testDef.m_InputVarToAnnotationData.Add(PropertyName, varAnnotationData);
				}
				else
				{
					testDef.m_OutputVarToAnnotationData.Add(PropertyName, varAnnotationData);
				}
			}
		}

		outTestsAndAnnotations.Add(ActorClassName, testDef);
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
	AActor* actor = testAnnotations.m_spawnedTestActorForTest;
	AEBLTTestTemplate* actorBaseTemplate = Cast<AEBLTTestTemplate>(actor);
	ensure(actorBaseTemplate);

	switch(strategy)
	{
	case TestParamsSuggestionStrategy::TESTPARAMSTRATEGY_RANDOM:
		{
			// TODO: here we should select which actors...maybe there are more that we can do !
			//TArray<AActor*> outAllActors;
			//UGameplayStatics::GetAllActorsOfClass(worldContext, testAnnotations.m_classToTest, outAllActors); //testAnnotations.m_classToTest, outAllActors);


			// For each actor, just randomize properties according to the annotations
//			for (const AActor* actor : outAllActors)
			
			{
				for (const TPair<FString, IGenericVarAnnotation*>& varSpec : testAnnotations.m_InputVarToAnnotationData)
				{
					IGenericVarAnnotation* varAnnotation = varSpec.Value;

					
					switch (varAnnotation->GetTestVariableType())
					{
						case TestVariableType::TEST_VAR_VECTOR:
						{
							const VarAnnotation_Vector* asVector = (VarAnnotation_Vector*)varAnnotation;
							const FVector3f val = asVector->generateRandomValue();
							const FStructProperty* propRef = CastField<FStructProperty>(varAnnotation->m_parentUEPropertyRef);


							FVector * targetValPtr = (FVector*) propRef->ContainerPtrToValuePtr<FVector>(actor);
							targetValPtr->X = val.X;
							targetValPtr->Y = val.Y;
							targetValPtr->Z = val.Z;


							actorBaseTemplate->debugSetVarValue(varSpec.Key, val.ToString());
						}
						break;

						case TestVariableType::TEST_VAR_FLOAT:
						{
							const VarAnnotation_Float* asFloat = (VarAnnotation_Float*)varAnnotation;
							const float val = asFloat->generateRandomValue();
							const FFloatProperty* propRef = CastField<FFloatProperty>(varAnnotation->m_parentUEPropertyRef);
							if (propRef)
							{
								float* targetValPtr = (float*)propRef->ContainerPtrToValuePtr<float>(actor);
								propRef->SetFloatingPointPropertyValue(targetValPtr, (double)val);
							}
							else
							{
								const FDoubleProperty* propRef_asDouble = CastField<FDoubleProperty>(varAnnotation->m_parentUEPropertyRef);

								if (propRef_asDouble)
								{
									double* targetValPtr = (double*)propRef_asDouble->ContainerPtrToValuePtr<double>(actor);
									propRef_asDouble->SetFloatingPointPropertyValue(targetValPtr, (double)val);

								}
							}
							
							actorBaseTemplate->debugSetVarValue(varSpec.Key, FString::SanitizeFloat(val));
						}
						break;

						case TestVariableType::TEST_VAR_INTEGER:
						{
							const VarAnnotation_Int* asInt = (VarAnnotation_Int*)varAnnotation;
							const int val = asInt->generateRandomValue();
							const FIntProperty* propRef = CastField<FIntProperty>(varAnnotation->m_parentUEPropertyRef);

							int* targetValPtr = (int*)propRef->ContainerPtrToValuePtr<int>(actor);
							propRef->SetFloatingPointPropertyValue(targetValPtr, val);

							actorBaseTemplate->debugSetVarValue(varSpec.Key, FString::FromInt(val));

						}
						break;

						case TestVariableType::TEST_VAR_OBJECT:
						{
							const VarAnnotation_Entity* asEntity = (VarAnnotation_Entity*)varAnnotation;
							const AActor* val = asEntity->generateRandomValue();
							const FObjectProperty* propRef = CastField<FObjectProperty>(varAnnotation->m_parentUEPropertyRef);

							const AActor* const* targetValPtr = propRef->ContainerPtrToValuePtr<AActor*>(actor);
							*((AActor**)targetValPtr) = (AActor * )val;

							actorBaseTemplate->debugSetVarValue(varSpec.Key, val->GetActorNameOrLabel());
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

	actorBaseTemplate->Internal_SetupContext();

	return true;
}

bool IGenericVarAnnotation::isFailOutputCondition() const
{
	// Some sanity checks first
	ensureMsgf(m_outputCheckType == VariableCheckType::VARCHECK_INVALID, TEXT("INvalid variable treated as output"));
	ensureMsgf(((m_failCode != VAR_ANNOTATION_INVALID_RESULT) ^ (m_succedCode != VAR_ANNOTATION_INVALID_RESULT)) == 1, TEXT("invalid succeed / fail codes"));

	if (m_failCode != VAR_ANNOTATION_INVALID_RESULT)
		return true;

	return false;
}



#pragma optimize("", on)

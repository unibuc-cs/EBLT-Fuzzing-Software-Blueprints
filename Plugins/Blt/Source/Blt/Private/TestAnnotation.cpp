#include "TestAnnotation.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Logging/LogMacros.h"
#include <algorithm>

#include "EBltBPLibrary.h"

#include "Containers/StringConv.h"
#pragma optimize("", off)


DEFINE_LOG_CATEGORY(LogBlt);

TestAnnotation_Float::TestAnnotation_Float(const std::string& genericAnnotation)
{
	m_testVariableType = TestVariableType::TEST_VAR_FLOAT;
	readAllValues(genericAnnotation);
}

float TestAnnotation_Float::generateRandomValue()
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
	sscanf_s(singleValueStr.c_str(), "%f", &res);
	return res;
}

TestAnnotation_Int::TestAnnotation_Int(const std::string& genericAnnotation)
{
	m_testVariableType = TestVariableType::TEST_VAR_INTEGER;

	readAllValues(genericAnnotation);
}

int TestAnnotation_Int::generateRandomValue()
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
	sscanf_s(singleValueStr.c_str(), "%d", &res);
	return res;
}

TestAnnotation_Vector::TestAnnotation_Vector(const std::string& genericAnnotation)
{
	m_testVariableType = TestVariableType::TEST_VAR_VECTOR;

	readAllValues(genericAnnotation);
}

FVector3f TestAnnotation_Vector::generateRandomValue()
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
	sscanf_s(singleValueStr.c_str(), "(%f,%f,%f)", &res.X, &res.Y, &res.Z);
	return res;
}

bool TestsAnnotationsParser::ParseTestsAnnotationsFromJSon(const FString& FilePath, MapFromTestNameToAnnotations& outTestsAndAnnotations)
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
	for (const TTuple<FString, TSharedPtr<FJsonValue>>& JsonClass : JsonClasses)
	{
		const FString& ActorClassName = JsonClass.Key;
		const UClass* const& BlueprintToTestClassType = UEBltBPLibrary::FindClass(ActorClassName);
		if (!BlueprintToTestClassType)
			continue;

		const TSharedPtr<FJsonObject>* ActorClassObject;
		if (!JsonClass.Value->TryGetObject(ActorClassObject))
		{
			UE_LOG(LogBlt, Error, TEXT("Entry %s must have an Object type value!"), *ActorClassName);
			continue;
		}

		const TMap<FString, TSharedPtr<FJsonValue>>& BlueprintToTestPropertiesSpec = ActorClassObject->Get()->Values;

		// TODO: parse from here the annotations
#if 0
		const bool& bIncludeBase = FuzzingFlags & static_cast<uint8>(EFuzzingFlags::IncludeBase);
		const bool& bIncludeSuper = FuzzingFlags & static_cast<uint8>(EFuzzingFlags::IncludeSuper);
		const bool& bIncludeNull = FuzzingFlags & static_cast<uint8>(EFuzzingFlags::IncludeNull);
#endif
		const bool bIncludeBase = true, bIncludeSuper = true , bIncludeNull = true;

		for (TFieldIterator<FProperty> Iterator(BlueprintToTestClassType); Iterator; ++Iterator)
		{
			const FProperty* const Property = *Iterator;
			const FString& PropertyName = Property->GetNameCPP();

			if (!BlueprintToTestPropertiesSpec.Contains(PropertyName))
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



#if 0
		TArray<AActor*> outActorsByClassName;
		UEBltBPLibrary::GetAllActorsOfClass(GetWorldForTests(), ActorClassName, outActorsByClassName);

		for (AActor* const actorTarget : outActorsByClassName)
		{
			UEBltBPLibrary::RandomiseProperties(actorTarget, JsonActorClassType, ActorClassProperties, (int32)EFuzzingFlags::All);
		}
	}
#endif
	}

	return true;
}

#pragma optimize("", on)

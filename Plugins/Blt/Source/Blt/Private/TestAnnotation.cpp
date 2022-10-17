#include "TestAnnotation.h"
#include <algorithm>

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
	sscanf(singleValueStr.c_str(), "%f", &res);
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
	sscanf(singleValueStr.c_str(), "%d", &res);
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
	sscanf(singleValueStr.c_str(), "(%f,%f,%f)", &res.X, &res.Y, &res.Z);
	return res;
}

bool TestsAnnotationsParser::ParseTestsAnnotationsFromJSon(const FString& FilePath)
{
	FString AbsoluteFilePath;
	if (!GetAbsolutePath(FilePath, AbsoluteFilePath))
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
		const UClass* const& JsonActorClassType = UEBltBPLibrary::FindClass(ActorClassName);
		if (!JsonActorClassType)
			continue;

		const TSharedPtr<FJsonObject>* ActorClassObject;
		if (!JsonClass.Value->TryGetObject(ActorClassObject))
		{
			UE_LOG(LogBlt, Error, TEXT("Entry %s must have an Object type value!"), *ActorClassName);
			continue;
		}

		const TMap<FString, TSharedPtr<FJsonValue>>& ActorClassProperties = ActorClassObject->Get()->Values;

		// TODO: parse from here the annotations

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

bool TestsAnnotationsParser::GetAbsolutePath(const FString& FilePath, FString& AbsoluteFilePath)
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

#pragma optimize("", on)

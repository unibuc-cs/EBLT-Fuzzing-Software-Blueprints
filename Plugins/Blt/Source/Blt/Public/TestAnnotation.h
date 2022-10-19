// This class handles JSON parsing and functionality for defining test annotations in a organized way
#pragma once
#include <string>
#include <vector>

#include "Misc/AssertionMacros.h"
#include "EBLTCommonUtils.h"
#include "UObject/Object.h"

#pragma optimize("", off)

class AActor;

enum class TestVariableType : int8_t
{
	TEST_VAR_DONOTTEST, // Leave as default
	TEST_VAR_INTEGER,
	TEST_VAR_FLOAT,
	TEST_VAR_VECTOR,
	TEST_VAR_STRING,
	TEST_VAR_OBJECT, // Generic ue object or other external lib base class
};

enum class VariableAnnotationType : int8_t
{
	VARANNOTATION_INVALID,
	VARANNOTATION_AS_SET, // Values are given in a possible set of values
	VARANNOTATIONS_AS_RANGE, // As range of values
};

class IGenericTestAnnotation : public UObject
{
public:
	const FProperty* m_parentUEPropertyRef = nullptr;

	virtual bool Init(const std::string& genericAnnotation) = 0;
	
	virtual const bool IsValid() const { return m_isValid; }

	virtual TestVariableType GetTestVariableType() const { return m_testVariableType; }

protected:
	bool m_isValid = false;
	TestVariableType m_testVariableType = TestVariableType::TEST_VAR_DONOTTEST;
};

template <typename T>
class ITestAnnotation : public IGenericTestAnnotation
{

	template< size_t N >
	static constexpr size_t length(char const (&)[N])
	{
		return N - 1;
	}

	static constexpr char GItems_Separator_Chr = '#';
	static constexpr char GItems_Min_Str[] = "min=";
	static constexpr char GItems_Max_Str[] = "max=";

	static constexpr int GItems_MinMax_Size = length(GItems_Min_Str);

public:
	virtual ~ITestAnnotation() {}
	virtual T generateRandomValue() const = 0;

	virtual bool readAllValues(const std::string& genericAnnotation)
	{
		std::string genericAnnotation_copy = genericAnnotation;
		EBLTCommonUtils::leftTrim(genericAnnotation_copy);
		EBLTCommonUtils::rightTrim(genericAnnotation_copy);

		// Set or range ? 
		if (genericAnnotation[0] == '{')
		{
			genericAnnotation_copy.erase(genericAnnotation_copy.begin());
			genericAnnotation_copy.erase(genericAnnotation_copy.end()-1);

			m_isValid = genericAnnotation_copy.size() > 0;

			std::string tempStr;
			for (int i = 0; i < genericAnnotation_copy.size(); i++)
			{
				if (genericAnnotation_copy[i] != GItems_Separator_Chr)
				{
					tempStr += genericAnnotation_copy[i];
				}
				else
				{
					const T res = readSingleValue(tempStr);
					m_setOfValues.push_back(res);
					tempStr.clear();
				}
			}

			if (tempStr.size() > 0)
			{
				const T res = readSingleValue(tempStr);
				m_setOfValues.push_back(res);
				tempStr.clear();
			}

			m_eAnnotationType = VariableAnnotationType::VARANNOTATION_AS_SET;
		}
		else if (genericAnnotation_copy[0] == '[')
		{
			m_isValid = genericAnnotation_copy.size() > 0;


			std::size_t minValuePos = genericAnnotation_copy.find(GItems_Min_Str);
			std::size_t maxValuePos = genericAnnotation_copy.find(GItems_Max_Str);

			ensureMsgf(minValuePos != std::string::npos && maxValuePos != std::string::npos, TEXT("Missing min and or max from def"));

			std::string minVal = std::string(genericAnnotation_copy.begin() + minValuePos + GItems_MinMax_Size,
				genericAnnotation_copy.begin() + maxValuePos);

			m_minVal = readSingleValue(minVal);

			std::string maxVal = std::string(genericAnnotation_copy.begin() + maxValuePos + GItems_MinMax_Size,
				genericAnnotation_copy.end());
			m_maxVal = readSingleValue(maxVal);

			m_eAnnotationType = VariableAnnotationType::VARANNOTATIONS_AS_RANGE;
		}

		return m_isValid;
	}

	virtual T readSingleValue(std::string& singleValueStr)
	{
		EBLTCommonUtils::leftTrim(singleValueStr);
		EBLTCommonUtils::rightTrim(singleValueStr);

		return T();
	}


	VariableAnnotationType GetType() const { return m_eAnnotationType; }
	T GetMinVal() const { return m_minVal; }
	T GetMaxVal() const { return m_maxVal; }
	const std::vector<T>& GetValues() const{ return m_setOfValues; }

protected:
	VariableAnnotationType m_eAnnotationType = VariableAnnotationType::VARANNOTATION_INVALID;

	// Either one of these will be used , probably could work as a union
	std::vector<T> m_setOfValues;
	T m_minVal;
	T m_maxVal;
};

class TestAnnotation_Float : public ITestAnnotation<float>
{
public:
	TestAnnotation_Float() {}
	virtual bool Init(const std::string& genericAnnotation) override;

	virtual float generateRandomValue() const override;
protected:
	// Reads from a string a concrete value
	virtual float readSingleValue(std::string& singleValueStr) override;
};

class TestAnnotation_Int : public ITestAnnotation<int>
{
public:
	TestAnnotation_Int();
	virtual bool Init(const std::string& genericAnnotation) override;

	virtual int generateRandomValue() const override;
protected:
	// Reads from a string a concrete value
	virtual int readSingleValue(std::string& singleValueStr) override;
};

class TestAnnotation_Vector : public ITestAnnotation<FVector3f>
{
public:
	TestAnnotation_Vector(){}
	virtual bool Init(const std::string& genericAnnotation) override;

	virtual FVector3f generateRandomValue() const override;

protected:
	// Reads from a string a concrete value
	virtual FVector3f readSingleValue(std::string& singleValueStr) override;
};


 enum class TestParamsSuggestionStrategy : uint8_t
 {
 	TESTPARAMSTRATEGY_RANDOM,
	TESTPARAMSSTRATEGY_RL 
 };

// This class contains all variables annotations inside a test
class SingleTestAnnotations
{
public:
	AActor* m_spawnedTestActorForTest = nullptr;
	UClass* m_classToTest = nullptr;
	TArray<AActor> m_allTestActors; // All actors of this class 
	TMap<FString, IGenericTestAnnotation*> m_VariableNameToAnnotationData;
};

using MapFromTestNameToAnnotations = TMap<FString, SingleTestAnnotations>;

class TestsAnnotationsHelper
{
public:
	// Builds specificiations about a sequence of tests and their variable ranges, inputs, outputs, etc
	static bool ParseTestsAnnotationsFromJSon(const FString& FilePath, MapFromTestNameToAnnotations& outTestsAndAnnotations);

	// Gets the properties and puts them in the target actor from a given strategy
	static bool BuildTestInstance(const UWorld* worldContext, const TestParamsSuggestionStrategy strategy, AActor* targetTestActor, const SingleTestAnnotations& testAnnotations);

private:

};

#pragma optimize("", on)

// This class handles JSON parsing and functionality for defining test annotations in a organized way
#pragma once
#include <string>
#include <vector>

#include "Misc/AssertionMacros.h"
#include "EBLTCommonUtils.h"

#pragma optimize("", off)

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

template <typename T>
class ITestAnnotation
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
	TestVariableType m_testVariableType = TestVariableType::TEST_VAR_DONOTTEST;

	virtual ~ITestAnnotation() {}
	virtual T generateRandomValue() = 0;

	virtual void readAllValues(const std::string& genericAnnotation)
	{
		std::string genericAnnotation_copy = genericAnnotation;
		EBLTCommonUtils::leftTrim(genericAnnotation_copy);
		EBLTCommonUtils::rightTrim(genericAnnotation_copy);

		// Set or range ? 
		if (genericAnnotation[0] == '{')
		{
			genericAnnotation_copy.erase(genericAnnotation_copy.begin());
			genericAnnotation_copy.erase(genericAnnotation_copy.end()-1);

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
	TestAnnotation_Float(const std::string& genericAnnotation);

	virtual float generateRandomValue() override;
protected:
	// Reads from a string a concrete value
	virtual float readSingleValue(std::string& singleValueStr) override;
};

class TestAnnotation_Int : public ITestAnnotation<int>
{
public:
	TestAnnotation_Int(const std::string& genericAnnotation);

	virtual int generateRandomValue() override;
protected:
	// Reads from a string a concrete value
	virtual int readSingleValue(std::string& singleValueStr) override;
};

class TestAnnotation_Vector : public ITestAnnotation<FVector3f>
{
public:
	TestAnnotation_Vector(const std::string& genericAnnotation);

	virtual FVector3f generateRandomValue() override;
protected:
	// Reads from a string a concrete value
	virtual FVector3f readSingleValue(std::string& singleValueStr) override;
};


// Holds specificiations about a sequence of tests and their variable ranges, inputs, outputs, etc
class TestsAnnotationsParser
{
public:
	bool ParseTestsAnnotationsFromJSon(const FString& FilePath);

private:

	bool GetAbsolutePath(const FString& FilePath, FString& AbsoluteFilePath);
};

#pragma optimize("", on)

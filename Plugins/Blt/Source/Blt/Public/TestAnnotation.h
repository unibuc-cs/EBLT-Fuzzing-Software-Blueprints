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

			std::size_t prevFound = 0;
			std::size_t found = genericAnnotation_copy.find(GItems_Separator_Chr, prevFound);
			while (found != std::string::npos)
			{
				// Process one item
				std::string nextItemStr = genericAnnotation_copy.substr(0, found);
				auto res = readSingleValue(nextItemStr);
				genericAnnotation_copy.erase(genericAnnotation_copy.begin(), genericAnnotation_copy.begin() + found);

				m_setOfValues.push_back(res);

				// then go to next item,
				found = genericAnnotation_copy.find(GItems_Separator_Chr);
			}

			m_eAnnotationType = VariableAnnotationType::VARANNOTATION_AS_SET;
		}
		else if (genericAnnotation_copy[0] == '[')
		{
			std::size_t minValuePos = genericAnnotation_copy.find(GItems_Min_Str);
			std::size_t maxValuePos = genericAnnotation_copy.find(GItems_Max_Str);

			ensureMsgf(minValuePos != std::string::npos && maxValuePos != std::string::npos, TEXT("Missing min and or max from def"));

			m_minVal = readSingleValue(std::string(genericAnnotation_copy.begin() + minValuePos + GItems_MinMax_Size,
				genericAnnotation_copy.begin() + maxValuePos));

			m_maxVal = readSingleValue(std::string(genericAnnotation_copy.begin() + maxValuePos + GItems_MinMax_Size,
				genericAnnotation_copy.end()));

			m_eAnnotationType = VariableAnnotationType::VARANNOTATIONS_AS_RANGE;
		}
	}

	virtual T readSingleValue(const std::string& singleValueStr) = 0;


	VariableAnnotationType GetType() { return m_eAnnotationType; }
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
	virtual float readSingleValue(const std::string& singleValueStr) override;
};

#pragma optimize("", on)
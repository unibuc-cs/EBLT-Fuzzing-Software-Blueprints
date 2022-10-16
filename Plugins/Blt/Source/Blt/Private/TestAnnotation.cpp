#include "TestAnnotation.h"
#include <algorithm>

#include "Containers/StringConv.h"
#pragma optimize("", off)

TestAnnotation_Float::TestAnnotation_Float(const std::string& genericAnnotation)
{
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

float TestAnnotation_Float::readSingleValue(const std::string& singleValueStr)
{
	float res = 0.0f;
	sscanf(singleValueStr.c_str(), "%f", &res);
	return res;
}

#pragma optimize("", on)
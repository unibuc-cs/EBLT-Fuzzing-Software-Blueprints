#include "TestAnnotation.h"
#include <algorithm>

#include "Containers/StringConv.h"


TestAnnotation_Float::TestAnnotation_Float(const std::string& genericAnnotation)
{
	readAllValues(genericAnnotation);
}

float TestAnnotation_Float::generateRandomValue()
{
	
}

float TestAnnotation_Float::readSingleValue(const std::string& singleValueStr)
{
	float res = 0.0f;
	sscanf(singleValueStr.c_str(), "%f", &res);
	return res;
}


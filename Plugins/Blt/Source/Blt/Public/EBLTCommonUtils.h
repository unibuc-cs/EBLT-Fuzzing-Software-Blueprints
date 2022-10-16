#pragma once

#include <string>
#include <algorithm>

#pragma optimize("", off)

class EBLTCommonUtils
{
public:


	// Cuts whitespaces at the beggining of a string
	static void leftTrim(std::string& str)
	{
		str.erase(str.begin(),
			std::find_if(str.begin(), str.end(), [](unsigned char ch) { return !std::isspace(ch); }));
	}

	// Similar as above but from the right side
	static void rightTrim(std::string& str)
	{
		auto firstSpacePos = std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base();
		str.erase(firstSpacePos, str.end());
	}
};

#pragma optimize("", on)
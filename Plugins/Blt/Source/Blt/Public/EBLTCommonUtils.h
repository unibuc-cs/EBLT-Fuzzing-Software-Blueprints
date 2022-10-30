#pragma once

#include <string>
#include <algorithm>
#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBlt, Log, All);


#pragma optimize("", off)


UENUM(BlueprintType)
enum class EBLTTestStatus : uint8 // \describes the lifecycle of a blueprint test
{
	EBLTTest_NotSetup, // Not supposed to run
	EBLTTest_NotRunning, // Can now check triggers
	EBLTTest_InProgress, // Test iis in progress
	EBLTTest_Failed,  // Test is finished with either failed or success
	EBLTTest_Success,
};


class EBLTCommonUtils
{
public:

	static constexpr const TCHAR* EnumToString(EBLTTestStatus testStatus)
	{
		switch(testStatus)
		{
		case EBLTTestStatus::EBLTTest_Success:
			return TEXT("TestSucceeded");
		default:
			return TEXT("TextFailed");
		}
	}

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

	// Gets the absolute path from a file in the Content dir.
	static bool GetAbsolutePath(const FString& FilePath, FString& AbsoluteFilePath);
};

#pragma optimize("", on)

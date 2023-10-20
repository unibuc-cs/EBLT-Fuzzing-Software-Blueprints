#include "EBLTCommonUtils.h"

#include "HAL/PlatformFileManager.h"
#include "Kismet/GameplayStatics.h"
#include "csv.hpp"//"csv-parser-master/include/csv.hpp"
#include "EBLTManager.h"
#include "Misc/Paths.h"

PRAGMA_DISABLE_OPTIMIZATION

bool EBLTCommonUtils::GetAbsolutePath(const FString& FilePath, FString& AbsoluteFilePath)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	AbsoluteFilePath = FilePath;
	if (PlatformFile.FileExists(*AbsoluteFilePath))
		return true;

	AbsoluteFilePath = FPaths::ProjectContentDir() + FilePath;
	if (PlatformFile.FileExists(*AbsoluteFilePath))
		return true;

	AbsoluteFilePath = "Game/"+ FilePath;
	if (PlatformFile.FileExists(*AbsoluteFilePath))
		return true;

	AbsoluteFilePath = AEBLTManager::getInstance()->GetPluginContentDir() + "/" + FilePath;
	if (PlatformFile.FileExists(*AbsoluteFilePath))
		return true;

	UE_LOG(LogBlt, Error, TEXT("File %s not found [relative path starts from the plugins Content folder]"), *AbsoluteFilePath);
	return false;
}

PRAGMA_ENABLE_OPTIMIZATION
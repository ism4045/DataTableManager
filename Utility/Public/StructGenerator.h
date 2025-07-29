// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"

#include "OpenXLSX.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include "Windows/HideWindowsPlatformTypes.h"
#endif
/**
 * 
 */
class DATATABLEMODULE_API StructGenerator
{
public:
	static bool GenerateStructFromXlsx(const FString& InXlsxFilePath, const FString& InCSVFolderPath, const FString& OutStructFolderPath);

	static bool WriteBasicInformation(std::ofstream& InOpenedFile);
	static bool WriteInclude(std::ofstream& InOpenedFile, const std::string& InHeaderName);
	static bool WriteStruct(std::ofstream& InOpenedFile, OpenXLSX::XLDocument& InOpenedDoc, const FString& InCSVFolderPath);

	static FString GetUnrealType(const FString& InVarType);
private:
	StructGenerator();
	~StructGenerator();
};

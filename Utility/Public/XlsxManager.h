// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"

#include "OpenXLSX.hpp"
#include <exception>
#include <sstream>
#include <algorithm>

#include "Windows/HideWindowsPlatformTypes.h"
#endif

#define TABLE_DIRECTORY "Table"
#define EXCEL_DIRECTORY "Excel"
#define CSV_DIRECTORY "CSV"
#define CSV_EXTENSION ".csv"

/**
 * 
 */
class DATATABLEMODULE_API XlsxManager
{
public:
	XlsxManager();
	~XlsxManager();

	static bool ConvertAllSheetInXlsx(const FString& InXlsxFilePath, const FString& OutCsvFolderPath);
	static bool ConvertSpecificSheet(const FString& InXlsxFilePath, const TArray<FString>& InSheetNames, const FString& OutCsvFolderPath);

	static void FindAllFilesInFolderPath(TArray<FString>& OutFilesPath, const FString& DirectoryPath, const FString& Extension);
	static void FindAllSheetInExcelFile(TArray<FString>& SheetNames, const FString& InXlsxFilePath);

	static bool CreateCSV(const OpenXLSX::XLWorksheet& InWorksheet, const FString& OutCsvFolderPath);
	static bool CheckIsDataTypeCell(std::string InStr);
};

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
class DATATABLEMODULE_API DataTableAssetGanerator
{
public:
	static bool CreateDataTableFromCSV(const FString& InAssetName, const FString& InCSVFilePath, const FString& InAssetFolderPath, TWeakObjectPtr<UScriptStruct> InStructObj);
private:
	DataTableAssetGanerator();
	~DataTableAssetGanerator();
};

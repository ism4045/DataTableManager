// Fill out your copyright notice in the Description page of Project Settings.

#include "XlsxManager.h"

using namespace OpenXLSX;
using namespace std;

static TArray<string> ValidType = { "int" , "uint", "int8", "uint8", "int16", "uint16", "int32", "uint32", "int64", "uint64", "float" , "double", "bool" , "boolean", "char" , "ansichar","tchar", "fstring" , "ftext","fname" };

XlsxManager::XlsxManager()
{
}

XlsxManager::~XlsxManager()
{
}

bool XlsxManager::ConvertAllSheetInXlsx(const FString& InXlsxFilePath, const FString& OutCsvFolderPath)
{
#if PLATFORM_WINDOWS
    // Check Valid Xlsx File Path
    if (InXlsxFilePath.IsEmpty() || FPaths::FileExists(InXlsxFilePath) == false)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid file path for XLSX"));
        return false;
    }
    //Check Valid CSV Folder Path
    if (OutCsvFolderPath.IsEmpty() || FPaths::DirectoryExists(OutCsvFolderPath) == false)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid Folder path for CSV"));
        return false;
    }

    try
    {
        XLDocument Doc;
        Doc.open(TCHAR_TO_UTF8(*InXlsxFilePath));

        //Get all Sheet's name in xlsx file
        vector<string> WorkSheetNames = Doc.workbook().worksheetNames();

        for (int Num = 0; Num < WorkSheetNames.size(); Num++)
        {
            XLWorksheet Wks = Doc.workbook().worksheet(WorkSheetNames[Num]);

            bool result = CreateCSV(Wks, OutCsvFolderPath);

            if (result == false)
            {
                return false;
            }
        }

        Doc.close();

        UE_LOG(LogTemp, Display, TEXT("Success to create Csv file on all sheet"));
        return true;
    }
    catch (const exception& e)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create Csv file : %s"), *FString(e.what()));
        return false;
    }
#endif
    UE_LOG(LogTemp, Error, TEXT("This feature is only available on Windows operating systems."));
    return false;
}

bool XlsxManager::ConvertSpecificSheet(const FString& InXlsxFilePath, const TArray<FString>& InSheetNames, const FString& OutCsvFolderPath)
{
#if PLATFORM_WINDOWS
    // Check Valid Xlsx File Path
    if (InXlsxFilePath.IsEmpty() || FPaths::FileExists(InXlsxFilePath) == false)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid file path for XLSX"));
        return false;
    }
    //Check Valid CSV Folder Path
    if (OutCsvFolderPath.IsEmpty() || FPaths::DirectoryExists(OutCsvFolderPath) == false)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid Folder path for CSV"));
        return false;
    }

    try
    {
        XLDocument Doc;
        Doc.open(TCHAR_TO_UTF8(*InXlsxFilePath));

        //Get all Sheet's name in xlsx file
        vector<string> WorkSheetNames = Doc.workbook().worksheetNames();

        for (int Num = 0; Num < WorkSheetNames.size(); Num++)
        {
            if (InSheetNames.Contains(WorkSheetNames[Num].c_str()))
            {
                XLWorksheet Wks = Doc.workbook().worksheet(WorkSheetNames[Num]);

                bool result = CreateCSV(Wks, OutCsvFolderPath);

                if (result == false)
                {
                    return false;
                }
            }
        }

        UE_LOG(LogTemp, Display, TEXT("Success to create Csv file on all sheet"));

        Doc.close();

        return true;
    }
    catch (const exception& e)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create Csv file : %s"), *FString(e.what()));
        return false;
    }
#endif
    UE_LOG(LogTemp, Error, TEXT("This feature is only available on Windows operating systems."));
    return false;
}

void XlsxManager::FindAllFilesInFolderPath(TArray<FString>& OutFilesPath, const FString& FolderPath, const FString& Extension)
{
    if (Extension.IsEmpty() || Extension.Len() < 2 || Extension.StartsWith(".") == false)
    {
        UE_LOG(LogTemp, Error, TEXT("Wrong Extension String"));
        return;
    }

    if (FolderPath.IsEmpty() || FPaths::DirectoryExists(FolderPath) == false)
    {
        UE_LOG(LogTemp, Error, TEXT("Folder is not exist"));
        return;
    }

    IFileManager& FileManager = IFileManager::Get();

    FString FindExtension = TEXT("*") + Extension;

    FileManager.FindFilesRecursive(
        OutFilesPath,
        *FolderPath,
        *FindExtension,
        true,
        true);
}

void XlsxManager::FindAllSheetInExcelFile(TArray<FString>& SheetNames, const FString& InXlsxFilePath)
{
#if PLATFORM_WINDOWS
    if (FPaths::FileExists(InXlsxFilePath) == false)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid file path for XLSX"));
    }

    SheetNames.Empty();

    try
    {
        XLDocument Doc;
        Doc.open(TCHAR_TO_UTF8(*InXlsxFilePath));

        //Get all Sheet's name in xlsx file
        vector<string> WorkSheetNames = Doc.workbook().worksheetNames();

        for (int Num = 0; Num < WorkSheetNames.size(); Num++)
        {
            SheetNames.Add(WorkSheetNames[Num].c_str());
        }

        Doc.close();
    }
    catch (const exception& e)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to Search Csv file : %s"), *FString(e.what()));
    }
#endif
}

bool XlsxManager::CreateCSV(const XLWorksheet& InWorksheet, const FString& OutCsvFolderPath)
{
    FString CsvContent;

    bool bFindKeyData = false;

    int RowNum = 0;
    int StartRow = -1;
    int StartCell = -1;
    int KeyCell = -1;
    int KeyValue = 1;

    for (const auto& Row : InWorksheet.rows())
    {
        TArray<FString> RowValues;
        int CellNum = 0;
        
        for (const auto& Cell : Row.cells())
        {
            vector<string> StringParseAry;
            string Parse;
            stringstream Ss;
            Ss << Cell.value();

            if (StartRow == -1 || RowNum == StartRow)
            {
                while (getline(Ss, Parse, '='))
                {
                    StringParseAry.push_back(Parse);

                    if (CheckIsDataTypeCell(Parse) && StartRow == -1)
                    {
                        StartRow = RowNum;
                        StartCell = CellNum;
                    }

                    if (Parse == "KEY" && RowNum == StartRow)
                    {
                        bFindKeyData = true;
                        KeyCell = CellNum;
                    }
                }
            }

            if (StartRow <= RowNum && StartCell <= CellNum && StartRow != -1 && StartCell != -1)
            {
                if (RowNum == StartRow && StringParseAry.size() >= 1)
                {
                    RowValues.Add(FString(StringParseAry[0].c_str()));
                }
                else
                {
                    RowValues.Add(FString(Ss.str().c_str()));
                }
            }

            CellNum++;
        }

        if (StartRow <= RowNum && StartRow != -1)
        {
            if (StartRow == RowNum || StartRow == RowNum - 1)
            {
                RowValues.EmplaceAt(0, "Key");
            }
            else if (KeyCell == -1)
            {
                RowValues.EmplaceAt(0, FString::FromInt(KeyValue));
            }
            else
            {
                FString PutData = RowValues[KeyCell - StartCell];
                RowValues.EmplaceAt(0, PutData);
            }

            CsvContent += FString::Join(RowValues, TEXT(",")) + TEXT("\n");
        }

        RowNum++;
    }

    bool Result = FFileHelper::SaveStringToFile(CsvContent, *FPaths::Combine(OutCsvFolderPath, InWorksheet.name().append(CSV_EXTENSION).c_str()));

    if (Result == false)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create Csv file"));
        return false;
    }
    
    return true;
}

bool XlsxManager::CheckIsDataTypeCell(std::string InStr)
{
    std::transform(InStr.begin(), InStr.end(), InStr.begin(), ::tolower);

    return ValidType.Contains(InStr);
}

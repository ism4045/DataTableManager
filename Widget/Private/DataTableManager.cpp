#include "DataTableManager.h"

#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "HAL/FileManager.h"
#include "Containers/Map.h"
#include "Misc/ConfigCacheIni.h"

#include "FolderPathModifier.h"
#include "SheetListView.h"

#include "Editor.h"
#include "EditorStyleSet.h"
#include "EditorFontGlyphs.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Views/SHeaderRow.h"

#include "UObject/ObjectMacros.h"
#include "UObject/UObjectIterator.h"
#include "Engine/DataTable.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/UserDefinedStruct.h"

#include "XlsxManager.h"
#include "StructGenerator.h"
#include "DataTableAssetGenerator.h"

#define LOCTEXT_NAMESPACE "DataTableManager"

void SDataTableManager::Construct(const FArguments& InArgs)
{
    TryCacheUStructName();

    FolderPathModifiers.Add(EPathType::PATH_EXCEL, nullptr);
    FolderPathModifiers.Add(EPathType::PATH_CSV, nullptr);
    FolderPathModifiers.Add(EPathType::PATH_STRUCT, nullptr);
    FolderPathModifiers.Add(EPathType::PATH_ASSET, nullptr);

    this->ChildSlot
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(5)
                [
                    SAssignNew(FolderPathModifiers[EPathType::PATH_EXCEL],SFolderPathModifier)
                        .Title_Str(TEXT("Excel Folder Path"))
                        .Hint_Str(TEXT("Excel Folder Path (You Must be Select Folder in Project Directory)"))
                        .PathType(EPathType::PATH_EXCEL)
                        .DefaultPath(FPaths::ProjectDir())
                        .OnTextChanged(this, &SDataTableManager::OnExcelFolderPathTextChanged)
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(5)
                [
                    SAssignNew(FolderPathModifiers[EPathType::PATH_CSV], SFolderPathModifier)
                        .Title_Str(TEXT("CSV Folder Path"))
                        .Hint_Str(TEXT("CSV Folder Path (You Must be Select Folder in Project Directory)"))
                        .PathType(EPathType::PATH_CSV)
                        .DefaultPath(FPaths::ProjectDir())
                        .OnTextChanged(this, &SDataTableManager::OnCSVFolderPathTextChanged)
                ]
                + SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)
				[
                    SAssignNew(FolderPathModifiers[EPathType::PATH_STRUCT], SFolderPathModifier)
                        .Title_Str(TEXT("Struct Folder Path"))
                        .Hint_Str(TEXT("Struct Header Folder Path (You Must be Select Folder in Project Source Directory)"))
                        .PathType(EPathType::PATH_STRUCT)
                        .DefaultPath(FPaths::GameSourceDir())
				]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(5)
                [
                    SAssignNew(FolderPathModifiers[EPathType::PATH_ASSET], SFolderPathModifier)
                        .Title_Str(TEXT("Asset Folder Path"))
                        .Hint_Str(TEXT("Struct Header Folder Path (You Must be Select Folder in Project Source Directory)"))
                        .PathType(EPathType::PATH_ASSET)
                        .DefaultPath(FPaths::ProjectContentDir())
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 5, 0, 0)
                [
                    SAssignNew(SheetListView, SSheetListView)
                ]
				+ SVerticalBox::Slot()
				.AutoHeight()
                .Padding(0, 5, 0, 0)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						[
                            CreateControlButton("Generate CSV in CSV folder path", &SDataTableManager::OnCSVConverterClicked)
						]
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .VAlign(VAlign_Center)
                        .HAlign(HAlign_Center)
                        [
                            CreateControlButton("Generate a struct header file in the structure folder through all the sheets in the Excel file in the selected sheet", &SDataTableManager::OnStructGeneratorClicked)
                        ]
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .VAlign(VAlign_Center)
                        .HAlign(HAlign_Center)
                        [
                            CreateControlButton("Import CSV to Data table in data table asset folder path", &SDataTableManager::OnImportCSVClicked)
                        ]
				]
        ];

    TryCacheExcelFiles();
    TryCacheCSVFiles();
    SheetListView->RefreshSheetState(ExcelFiles, CSVFiles, UStructObjs);
    SheetListView->RefreshCSVState(CSVFiles);
}

void SDataTableManager::OnCSVFolderPathTextChanged(const FText& NewText)
{
    TryCacheCSVFiles();
    SheetListView->RefreshCSVState(CSVFiles);
}

void SDataTableManager::OnExcelFolderPathTextChanged(const FText& NewText)
{
    TryCacheExcelFiles();
    TryCacheCSVFiles();
    SheetListView->RefreshSheetState(ExcelFiles, CSVFiles, UStructObjs);
    SheetListView->RefreshCSVState(CSVFiles);
}

void SDataTableManager::TryCacheUStructName()
{
    UStructObjs.Empty();

    for (TObjectIterator<UScriptStruct> It; It; ++It)
    {
        UScriptStruct* Struct = *It;
        if (Struct->IsChildOf(FTableRowBase::StaticStruct()))
        {
            if (Struct != FTableRowBase::StaticStruct())
            {
                UStructObjs.Add(Struct);
            }
        }
    }
}

void SDataTableManager::TryCacheCSVFiles()
{
    CSVFiles.Empty();

    XlsxManager::FindAllFilesInFolderPath(CSVFiles, *FolderPathModifiers[EPathType::PATH_CSV]->GetPathPtrOnType(), ".csv");
}

void SDataTableManager::TryCacheExcelFiles()
{
    ExcelFiles.Empty();

    XlsxManager::FindAllFilesInFolderPath(ExcelFiles, *FolderPathModifiers[EPathType::PATH_EXCEL]->GetPathPtrOnType(), ".xlsx");
}


TSharedRef<SWidget> SDataTableManager::CreateControlButton(const FString& InBtnText, FReply(SDataTableManager::* InFunc)())
{
    return SNew(SBox)
        .HeightOverride(50.f)
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Fill)
        [
            SNew(SButton).VAlign(VAlign_Center).HAlign(HAlign_Center)
                .Text(FText::FromString(InBtnText))
                .OnClicked(this, InFunc)
        ];
}


FReply SDataTableManager::OnCSVConverterClicked()
{
    FString CSVFolderPath = *FolderPathModifiers[EPathType::PATH_CSV]->GetPathPtrOnType();

    if (CSVFolderPath.IsEmpty() || FPaths::DirectoryExists(CSVFolderPath) == false)
    {
        FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, LOCTEXT("ErrorMSG_FolderPath", "CSV Folder Path is not exist or invalid"));
        return FReply::Handled();
    }

    TMap<FString,TArray<FString>> SheetMap;

    for (TSharedPtr<FSheetListRowData> Data : SheetListView->GetDataList())
    {
        if (Data.IsValid() && Data->IsChecked())
        {
            SheetMap.FindOrAdd(Data->GetExcelFullPath()).Add(Data->GetSheetName());
        }
    }

    for (TMap<FString, TArray<FString>>::TConstIterator Iter = SheetMap.CreateConstIterator(); Iter; ++Iter)
    {
        const FString& ExcelFullPath = Iter.Key();
        const TArray<FString>& SheetNames = Iter.Value();

        bool Result = XlsxManager::ConvertSpecificSheet(ExcelFullPath, SheetNames, CSVFolderPath);

        if (Result == false)
        {
            FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, LOCTEXT("ErrorMSG_ConvertCSV", "Convert CSV Failed"));
        }
    }

    SheetListView->RefreshCSVState(CSVFiles);

    FMessageDialog::Open(EAppMsgCategory::Success, EAppMsgType::Ok, LOCTEXT("SuccessMSG_ConvertCSV", "Convert CSV Success"));

    return FReply::Handled();
}

FReply SDataTableManager::OnStructGeneratorClicked()
{
    FString CSVFolderPath = *FolderPathModifiers[EPathType::PATH_CSV]->GetPathPtrOnType();
    FString StructFolderPath = *FolderPathModifiers[EPathType::PATH_STRUCT]->GetPathPtrOnType();

    if (CSVFolderPath.IsEmpty() || FPaths::DirectoryExists(CSVFolderPath) == false)
    {
        FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, LOCTEXT("ErrorMSG_FolderPath", "CSV Folder Path is not exist or invalid"));
        return FReply::Handled();
    }

    if (StructFolderPath.IsEmpty() || FPaths::DirectoryExists(StructFolderPath) == false)
    {
        FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, LOCTEXT("ErrorMSG_FolderPath", "Struct Folder Path is not exist or invalid"));
        return FReply::Handled();
    }

    TArray<FString> ExcelAry;

    for (TSharedPtr<FSheetListRowData> Data : SheetListView->GetDataList())
    {
        if (Data.IsValid() && Data->IsChecked())
        {
            if (Data->IsExistCSV() == false)
            {
                FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, LOCTEXT("ErrorMSG_ExistCSV", "There is an item in the selected sheet where the csv file does not exist"));
                return FReply::Handled();
            }
            else
            {
                ExcelAry.AddUnique(Data->GetExcelFullPath());
            }
        }
    }

    bool Result = true;

    for (const FString& ExcelPath : ExcelAry)
    {
        Result = StructGenerator::GenerateStructFromXlsx(ExcelPath, CSVFolderPath, StructFolderPath);

        if (Result == false)
        {
            break;
        }
    }

    if (Result)
    {
        FMessageDialog::Open(EAppMsgCategory::Success, EAppMsgType::Ok, LOCTEXT("SuccessMSG_GenerateStruct", "Generate Struct Success\nTo import csv, please rebuild the project"));
    }
    else
    {
        FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, LOCTEXT("SuccessMSG_GenerateStruct", "Generate Struct Failed"));
    }

    return FReply::Handled();
}

FReply SDataTableManager::OnImportCSVClicked()
{
    FString AssetFolderPath = *FolderPathModifiers[EPathType::PATH_ASSET]->GetPathPtrOnType();

    if (AssetFolderPath.IsEmpty() || FPaths::DirectoryExists(AssetFolderPath) == false)
    {
        FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, LOCTEXT("ErrorMSG_FolderPath", "Asset Folder Path is not exist or invalid"));
        return FReply::Handled();
    }

    bool Result = false;

    for (TSharedPtr<FSheetListRowData> Data : SheetListView->GetDataList())
    {
        if (Data.IsValid() && Data->IsChecked())
        {
            Result = DataTableAssetGanerator::CreateDataTableFromCSV(Data->GetSheetName(), Data->GetCSVFullPath(), AssetFolderPath, Data->GetStructure());

            if (Result == false)
            {
                break;
            }
        }
    }

    if (Result)
    {
        FMessageDialog::Open(EAppMsgCategory::Success, EAppMsgType::Ok, LOCTEXT("SuccessMSG_CreateAsset", "Create Data Table Success"));
    }
    else
    {
        FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, LOCTEXT("ErrorMSG_CreateAsset", "Create Data Table Failed"));
    }

    return FReply::Handled();
}

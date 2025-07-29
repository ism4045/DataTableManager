// Fill out your copyright notice in the Description page of Project Settings.

#include "DataTableAssetGenerator.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Factories/DataTableFactory.h"
#include "AssetToolsModule.h"
#include "UObject/SavePackage.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Engine.h"



bool DataTableAssetGanerator::CreateDataTableFromCSV(const FString& InAssetName, const FString& InCSVFilePath, const FString& InAssetFolderPath, TWeakObjectPtr<UScriptStruct> InStructObj)
{
    bool bExistFile = InCSVFilePath.IsEmpty() == false && FPaths::FileExists(InCSVFilePath);
    bool bExistFolder = InAssetFolderPath.IsEmpty() == false && FPaths::DirectoryExists(InAssetFolderPath);

    if (bExistFile == false || bExistFolder == false || InStructObj.IsValid() == false)
    {
        return false;
    }

    FString AssetPath;
    FPackageName::TryConvertFilenameToLongPackageName(InAssetFolderPath, AssetPath);

    FString CSVStr;
    FFileHelper::LoadFileToString(CSVStr, *InCSVFilePath);

    TArray<FString> Lines;
    CSVStr.ParseIntoArrayLines(Lines, true);
    if (Lines.IsValidIndex(0))
    {
        Lines.RemoveAt(0);
        CSVStr = FString::Join(Lines, TEXT("\n"));
    }

    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

    UDataTableFactory* DataTableFactory = NewObject<UDataTableFactory>();
    DataTableFactory->Struct = InStructObj.Get();

	const FString& PackageName = FPaths::Combine(AssetPath, InAssetName);
	const FString& AssetName = InAssetName;

    UObject* DataTableAsset = LoadObject<UObject>(nullptr, *PackageName);
    if (IsValid(DataTableAsset) == false)
    {
        DataTableAsset = AssetToolsModule.Get().CreateAsset(AssetName, FPaths::GetPath(PackageName), UDataTable::StaticClass(), DataTableFactory);
    }

    if (IsValid(DataTableAsset))
    {
        UDataTable* NewDataTable = Cast<UDataTable>(DataTableAsset);
        if (IsValid(NewDataTable))
        {
            TArray<FString> Problems = NewDataTable->CreateTableFromCSVString(CSVStr);
            if (Problems.Num() > 0)
            {
                for (const FString& Problem : Problems)
                {
                    UE_LOG(LogTemp, Error, TEXT("Problem importing DataTable '%s' : %s"), *DataTableAsset->GetName(), *Problem);
                }
            }
        }

        FAssetRegistryModule::AssetCreated(DataTableAsset);
        DataTableAsset->MarkPackageDirty();

        UPackage* Package = DataTableAsset->GetOutermost();
        FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
        FSavePackageArgs SaveArgs;
        SaveArgs.TopLevelFlags = EObjectFlags::RF_Public | EObjectFlags::RF_Standalone;

        return UPackage::SavePackage(Package, nullptr, *PackageFileName, SaveArgs);
    }

    return false;
}

DataTableAssetGanerator::DataTableAssetGanerator()
{

}

DataTableAssetGanerator::~DataTableAssetGanerator()
{
}
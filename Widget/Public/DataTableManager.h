#pragma once

#include "CoreMinimal.h"
#include "DataTableManagerConfig.h"
#include "Widgets/SCompoundWidget.h"

class SFolderPathModifier;
class SSheetListView;

class DATATABLEMODULE_API SDataTableManager : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SDataTableManager) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    void OnExcelFolderPathTextChanged(const FText& NewText);
    void OnCSVFolderPathTextChanged(const FText& NewText);

    // Create Button
    TSharedRef<SWidget> CreateControlButton(const FString& InBtnText, FReply(SDataTableManager::* InFunc)());
    FReply OnCSVConverterClicked();
    FReply OnStructGeneratorClicked();
    FReply OnImportCSVClicked();

    // Cached UStruct Based on FTableRowBase
    void TryCacheUStructName();
    void TryCacheCSVFiles();
    void TryCacheExcelFiles();

private:
    // Folder Path Modifier
    TMap<EPathType,TSharedPtr<SFolderPathModifier>> FolderPathModifiers;
    TSharedPtr<SSheetListView> SheetListView;

    TArray<TWeakObjectPtr<UScriptStruct>> UStructObjs;
    TArray<FString> CSVFiles;
    TArray<FString> ExcelFiles;
};

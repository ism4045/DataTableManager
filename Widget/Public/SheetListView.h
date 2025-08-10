#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

DECLARE_MULTICAST_DELEGATE(FDataChangedDelegate);

class SFolderPathModifier;
class SDataTableManager;

struct FindBySheetName_Functer
{
public:
    const FString& ExternalStr;

    FindBySheetName_Functer(const FString& InExternalStr) : ExternalStr(InExternalStr) {}

    bool operator()(const FString& Str) const
    {
        return FPaths::GetCleanFilename(Str) == (ExternalStr + ".csv");
    }

    bool operator()(const TWeakObjectPtr<UScriptStruct>& Obj) const
    {
        if (Obj.IsValid() == false)
        {
            return false;
        }
        return Obj->GetName() == ExternalStr;
    }
};

struct FSheetListRowData
{
public:

    static TSharedRef<FSheetListRowData> Make(
        FString SheetName,
        FString InExcelName,
        FString InExcelFullPath,
        bool InbExistCSV = false,
        FString InCSVFullPath = "",
        bool InbExistStruct = false,
        UScriptStruct* InStructObj = nullptr,
        bool bIsChecked = false)
    {
        return MakeShareable(new FSheetListRowData(SheetName, InExcelName, InExcelFullPath, InbExistCSV, InCSVFullPath, InbExistStruct, InStructObj, bIsChecked));
    }

    static TSharedRef<FSheetListRowData> Make(
        FString SheetName,
        FString InExcelFullPath,
        FString InCSVFullPath = "",
        UScriptStruct* InStructObj = nullptr,
        bool bIsChecked = false)
    {
        return MakeShareable(new FSheetListRowData(SheetName, InExcelFullPath, InCSVFullPath, InStructObj, bIsChecked));
    }

    FSheetListRowData(FString InSheetName, FString InExcelName, FString InExcelFullPath, bool InbExistCSV = false, FString InCSVFullPath = "", bool InbExistStruct = false, UScriptStruct* InStructObj = nullptr, bool InbIsChecked = false)
        : SheetName(InSheetName), ExcelName(InExcelName), ExcelFullPath(InExcelFullPath), bExistCSV(InbExistCSV), CSVFullPath(InCSVFullPath), bExistStruct(InbExistStruct), StructObj(InStructObj), bIsChecked(InbIsChecked)
    {
    }

    FSheetListRowData(FString InSheetName, FString InExcelFullPath, FString InCSVFullPath = "", UScriptStruct* InStructObj = nullptr, bool InbIsChecked = false)
        : SheetName(InSheetName), ExcelFullPath(InExcelFullPath), CSVFullPath(InCSVFullPath), StructObj(InStructObj), bIsChecked(InbIsChecked)
    {
        ExcelName = FPaths::GetCleanFilename(InExcelFullPath);
        bExistCSV = FPaths::FileExists(InCSVFullPath);
        bExistStruct = IsValid(InStructObj);
    }

    FString GetSheetName() const
    {
        return SheetName;
    }

    FString GetExcelName() const
    {
        return ExcelName;
    }

    FString GetExcelFullPath() const
    {
        return ExcelFullPath;
    }

    FString GetCSVFullPath() const
    {
        return CSVFullPath;
    }

    TWeakObjectPtr<UScriptStruct> GetStructure() const
    {
        return StructObj;
    }

    bool IsExistCSV() const
    {
        return bExistCSV;
    }

    bool IsExistStruct() const
    {
        return bExistStruct;
    }

    bool IsChecked() const
    {
        return bIsChecked;
    }

    void SetCSVFullPath(const FString& InPath, const bool& bIgnoreDelegate = false)
    {
        CSVFullPath = InPath;
        bool PrevData = bExistCSV;
        bExistCSV = FPaths::FileExists(CSVFullPath);

        if (bExistCSV != PrevData && bIgnoreDelegate == false)
        {
            DataChangedDelegate.Broadcast();
        }
    }

    void SetIsChecked(const bool& InIsChecked, const bool& bIgnoreDelegate = false)
    {
        bool PrevData = bIsChecked;
        bIsChecked = InIsChecked;

        if (bIsChecked != PrevData && bIgnoreDelegate == false)
        {
            DataChangedDelegate.Broadcast();
        }
    }

public:
    FDataChangedDelegate DataChangedDelegate;

protected:
    FString SheetName;

    FString ExcelName;
    FString ExcelFullPath;

    bool bExistCSV;
    FString CSVFullPath;

    bool bExistStruct;
    TWeakObjectPtr<UScriptStruct> StructObj = nullptr;

    bool bIsChecked;
};

class DATATABLEMODULE_API SSheetListRow : public SMultiColumnTableRow<TSharedPtr<FSheetListRowData>>
{
public:
    SLATE_BEGIN_ARGS(SSheetListRow) {}
        SLATE_ARGUMENT(TSharedPtr<FSheetListRowData>, Item)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

    virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

    void OnDataChanged();
    void OnCheckStateChanged(ECheckBoxState NewState);

private:
    TSharedRef<SWidget> GetWidgetForColum(const FName& ColumnName);

private:
    TSharedPtr<FSheetListRowData> Item;
    TSharedPtr<STextBlock> ExistCSVTextBlock;
    TSharedPtr<SCheckBox> CheckBox;
};

class DATATABLEMODULE_API SSheetListView : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SSheetListView) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

public:
    TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FSheetListRowData> InItem, const TSharedRef<STableViewBase>& OwnerTable);

    void OnCheckStateChanged(ECheckBoxState NewState);

    void RefreshSheetState(const TArray<FString>& ExcelFiles, const TArray<FString>& CSVFiles, TArray<TWeakObjectPtr<UScriptStruct>>& StructObjs);
    void RefreshCSVState(const TArray<FString>& CSVFiles);

    TArray<TSharedPtr<FSheetListRowData>>& GetDataList() { return DataList;  }

private:
    TSharedPtr<SListView<TSharedPtr<FSheetListRowData>>> ListView;
    TArray<TSharedPtr<FSheetListRowData>> DataList;

    TSharedPtr<SCheckBox> ControlCheckBox;
};


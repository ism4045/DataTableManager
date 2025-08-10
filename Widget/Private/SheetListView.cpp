#include "SheetListView.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Views/SHeaderRow.h"

#include "XlsxManager.h"
#include "DataTableManager.h"

#define LOCTEXT_NAMESPACE "DataTableManager"

static const FName ColumnID_SelectLabel("Select");
static const FName ColumnID_SheetLabel("SheetName");
static const FName ColumnID_ExcelLabel("ExcelName");
static const FName ColumnID_ExistCSVLabel("ExistCSV");
static const FName ColumnID_ExistStructLabel("ExistStruct");

void SSheetListRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
    Item = InArgs._Item;

    Item->DataChangedDelegate.AddSP(this, &SSheetListRow::OnDataChanged);

    SMultiColumnTableRow<TSharedPtr<FSheetListRowData>>::Construct(
        FSuperRowType::FArguments().Style(FAppStyle::Get(), "TableView.DarkRow"), InOwnerTableView
    );
}

TSharedRef<SWidget> SSheetListRow::GenerateWidgetForColumn(const FName& ColumnName)
{
    return SNew(SBox)
        .HeightOverride(30.f)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            GetWidgetForColum(ColumnName)
        ];
}


TSharedRef<SWidget> SSheetListRow::GetWidgetForColum(const FName& ColumnName)
{
    if (Item.IsValid() == false)
    {
        return SNullWidget::NullWidget;
    }

    if (ColumnName == ColumnID_SelectLabel)
    {
        return SAssignNew(CheckBox, SCheckBox)
            .IsChecked(ECheckBoxState::Unchecked)
            .OnCheckStateChanged(this, &SSheetListRow::OnCheckStateChanged);
    }

    if (ColumnName == ColumnID_SheetLabel)
    {
        return SNew(STextBlock).Text(FText::FromString(Item->GetSheetName()));
    }

    if (ColumnName == ColumnID_ExcelLabel)
    {
        return SNew(STextBlock).Text(FText::FromString(Item->GetExcelName()));
    }

    if (ColumnName == ColumnID_ExistCSVLabel)
    {
        return SAssignNew(ExistCSVTextBlock, STextBlock)
            .Text(FText::FromString(Item->IsExistCSV() ? "True" : "False"))
            .ColorAndOpacity(FSlateColor(Item->IsExistCSV() ? FLinearColor::Blue : FLinearColor::Red));
    }

    if (ColumnName == ColumnID_ExistStructLabel)
    {
        return SNew(STextBlock)
            .Text(FText::FromString(Item->IsExistStruct() ? "True" : "False"))
            .ColorAndOpacity(FSlateColor(Item->IsExistStruct() ? FLinearColor::Blue : FLinearColor::Red));
    }

    return SNullWidget::NullWidget;
}

void SSheetListRow::OnDataChanged()
{
    if (ExistCSVTextBlock.IsValid() == false || CheckBox.IsValid() == false || Item.IsValid() == false)
    {
        return;
    }

    ExistCSVTextBlock->SetText(FText::FromString(Item->IsExistCSV() ? "True" : "False"));
    ExistCSVTextBlock->SetColorAndOpacity(FSlateColor(Item->IsExistCSV() ? FLinearColor::Blue : FLinearColor::Red));

    CheckBox->SetIsChecked(Item->IsChecked() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
}

void SSheetListRow::OnCheckStateChanged(ECheckBoxState NewState)
{
    if (Item.IsValid() == false)
    {
        return;
    }

    Item->SetIsChecked(NewState == ECheckBoxState::Checked, true);
}

void SSheetListView::Construct(const FArguments& InArgs)
{
    this->ChildSlot
        [
            SAssignNew(ListView, SListView<TSharedPtr<FSheetListRowData>>)
                .OnGenerateRow(this, &SSheetListView::OnGenerateRowForList)
                .ListItemsSource(&DataList)
                .Visibility(EVisibility::Visible)
                .HeaderRow(
                    SNew(SHeaderRow)
                    + SHeaderRow::Column(ColumnID_SelectLabel)
                    .DefaultLabel(LOCTEXT("CSVConverter_SelectLabel", "Select"))
                    .ManualWidth(100.f)
                    .VAlignHeader(VAlign_Center)
                    .HAlignHeader(HAlign_Center)
                    .VAlignCell(VAlign_Center)
                    .HAlignCell(HAlign_Center)
                    [
                        SAssignNew(ControlCheckBox, SCheckBox)
                            .IsChecked(ECheckBoxState::Unchecked)
                            .OnCheckStateChanged(this, &SSheetListView::OnCheckStateChanged)
                    ]

                    + SHeaderRow::Column(ColumnID_SheetLabel)
                    .DefaultLabel(LOCTEXT("CSVConverter_SheetLabel", "Sheet Name"))
                    .ManualWidth(300.f)
                    .VAlignHeader(VAlign_Center)
                    .HAlignHeader(HAlign_Center)
                    .VAlignCell(VAlign_Center)
                    .HAlignCell(HAlign_Center)

                    + SHeaderRow::Column(ColumnID_ExcelLabel)
                    .DefaultLabel(LOCTEXT("CSVConverter_ExcelLabel", "Excel Name"))
                    .ManualWidth(300.f)
                    .VAlignHeader(VAlign_Center)
                    .HAlignHeader(HAlign_Center)
                    .VAlignCell(VAlign_Center)
                    .HAlignCell(HAlign_Center)

                    + SHeaderRow::Column(ColumnID_ExistCSVLabel)
                    .DefaultLabel(LOCTEXT("CSVConverter_ExistCSVLabel", "Exist CSV in Project"))
                    .ManualWidth(300.f)
                    .VAlignHeader(VAlign_Center)
                    .HAlignHeader(HAlign_Center)
                    .VAlignCell(VAlign_Center)
                    .HAlignCell(HAlign_Center)

                    + SHeaderRow::Column(ColumnID_ExistStructLabel)
                    .DefaultLabel(LOCTEXT("CSVConverter_ExistStructLabel", "Exist Struct DataType in Project"))
                    .ManualWidth(300.f)
                    .VAlignHeader(VAlign_Center)
                    .HAlignHeader(HAlign_Center)
                    .VAlignCell(VAlign_Center)
                    .HAlignCell(HAlign_Center)
                )
        ];
}

TSharedRef<ITableRow> SSheetListView::OnGenerateRowForList(TSharedPtr<FSheetListRowData> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
    check(InItem.IsValid());

    return SNew(SSheetListRow, OwnerTable)
        .Item(InItem);
}

void SSheetListView::OnCheckStateChanged(ECheckBoxState NewState)
{
    for (TSharedPtr<FSheetListRowData> Data : DataList)
    {
        if (Data.IsValid())
        {
            Data->SetIsChecked(NewState == ECheckBoxState::Checked);
        }
    }
}

void SSheetListView::RefreshSheetState(const TArray<FString>& ExcelFiles, const TArray<FString>& CSVFiles, TArray<TWeakObjectPtr<UScriptStruct>>& StructObjs)
{
    DataList.Empty();
    
    if (ControlCheckBox.IsValid())
    {
        ControlCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
    }

    for (const FString& ExcelFile : ExcelFiles)
    {
        TArray<FString> SheetsInExcel;
        XlsxManager::FindAllSheetInExcelFile(SheetsInExcel, ExcelFile);

        for (const FString& SheetName : SheetsInExcel)
        {
            const FString* CSVFullPath = CSVFiles.FindByPredicate(FindBySheetName_Functer(SheetName));
            const TWeakObjectPtr<UScriptStruct>* UStructObj = StructObjs.FindByPredicate(FindBySheetName_Functer(SheetName));

            DataList.Add(MakeShared<FSheetListRowData>(SheetName, ExcelFile, CSVFullPath == nullptr ? "" : *CSVFullPath, UStructObj == nullptr ? nullptr : (*UStructObj).Get(), false));
        }
    }

    ListView->RequestListRefresh();
}

void SSheetListView::RefreshCSVState(const TArray<FString>& CSVFiles)
{
    for (TSharedPtr<FSheetListRowData> Data : DataList)
    {
        if (Data.IsValid())
        {
            const FString* CSVFullPath = CSVFiles.FindByPredicate(FindBySheetName_Functer(Data->GetSheetName()));
            if (CSVFullPath != nullptr)
            {
                Data->SetCSVFullPath(*CSVFullPath);
            }
        }
    }
}

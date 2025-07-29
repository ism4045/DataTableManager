#include "DataTableManager.h"

#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "HAL/FileManager.h"
#include "Containers/Map.h"
#include "Misc/ConfigCacheIni.h"

#include "Editor.h"
#include "EditorStyleSet.h"
#include "EditorFontGlyphs.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "WIdgets/SOverlay.h"
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

static const float EditableTextBox_Length = 1125.f;
static const float TextBlock_Length = 100.f;

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
        return SAssignNew(ExistCSVTextBlock ,STextBlock)
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

void SDataTableManager::Construct(const FArguments& InArgs)
{
    ToolConfig = GetMutableDefault<UDataTableManagerConfig>();
    ToolConfig->LoadConfig();

    TryCacheUStructName();

    this->ChildSlot
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(5)
                [
                    CreateFolderPathModifier(TEXT("Excel Folder Path"),
                        TEXT("Excel Folder Path (You Must be Select Folder in Project Directory)"), ToolConfig->CachedExcelPath,
                        ExcelFolderPathWidget,
                        &SDataTableManager::OnExcelFolderPathTextChanged,
                        &SDataTableManager::ExcelFolderBrowserBtnClicked)
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(5)
                [
                    CreateFolderPathModifier(TEXT("CSV Folder Path"),
                        TEXT("CSV Folder Path (You Must be Select Folder in Project Directory)"), ToolConfig->CachedCSVPath,
                        CSVFolderPathWidget,
                        &SDataTableManager::OnCSVFolderPathTextChanged,
                        &SDataTableManager::CSVFolderBrowserBtnClicked)
                ]
                + SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)
				[
					CreateFolderPathModifier(TEXT("Struct Folder Path"),
						TEXT("Struct Header Folder Path (You Must be Select Folder in Project Source Directory)"), ToolConfig->CachedStructPath,
						StructFolderPathWidget,
						&SDataTableManager::OnStructFolderPathTextChanged,
						&SDataTableManager::StructFolderBrowserBtnClicked)
				]
                + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(5)
                    [
                        CreateFolderPathModifier(TEXT("Asset Folder Path"),
                            TEXT("Asset Folder Path (You Must be Select Folder in Project Content Directory)"), ToolConfig->CachedAssetPath,
                            AssetFolderPathWidget,
                            &SDataTableManager::OnAssetFolderPathTextChanged,
                            &SDataTableManager::AssetFolderBrowserBtnClicked)
                    ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 5, 0, 0)
                [
                    CreateListView()
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
    RefreshSheetState();
    RefreshCSVState();
}

TSharedRef<SWidget> SDataTableManager::CreateListView()
{
    return SAssignNew(this->ListView, SListView<TSharedPtr<FSheetListRowData>>)
        .OnGenerateRow(this, &SDataTableManager::OnGenerateRowForList)
        .ListItemsSource(&this->DataList)
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
                    .OnCheckStateChanged(this, &SDataTableManager::OnCheckStateChanged)
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
        );
}

TSharedRef<ITableRow> SDataTableManager::OnGenerateRowForList(TSharedPtr<FSheetListRowData> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
    check(InItem.IsValid());

    return SNew(SSheetListRow, OwnerTable)
        .Item(InItem);
}

TSharedRef<SWidget> SDataTableManager::CreateFolderPathModifier(const FString& InTitleStr, const FString& HintTextValue, const FString& TextBlockValue, TSharedPtr<SEditableTextBox>& InWidgetPtr, void(SDataTableManager::* InFunc_ETB)(const FText&), FReply(SDataTableManager::* InFunc_Btn)())
{
    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .HAlign(HAlign_Left)
        .Padding(6, 0)
        [
            CreateFolderPathTitle(InTitleStr)
        ]
        + SHorizontalBox::Slot()
        .FillWidth(1.0f)
        .MaxWidth(EditableTextBox_Length)
        .VAlign(VAlign_Center)
        .HAlign(HAlign_Left)
        .Padding(6, 0)
        [
            CreateFolderPathTextBox(HintTextValue, TextBlockValue, InWidgetPtr, InFunc_ETB)
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .HAlign(HAlign_Left)
        .Padding(6, 0)
        [
            CreateFolderBrowserButton(InFunc_Btn)
        ];
}

TSharedRef<SWidget> SDataTableManager::CreateFolderPathTitle(const FString& InTitleStr)
{
    return SNew(STextBlock)
        .Text(FText::FromString(InTitleStr))
        .MinDesiredWidth(TextBlock_Length)
        .ColorAndOpacity(FSlateColor(FLinearColor::White));
}


TSharedRef<SWidget> SDataTableManager::CreateFolderPathTextBox(const FString& HintTextValue, const FString& TextBlockValue, TSharedPtr<SEditableTextBox>& InWidgetPtr, void(SDataTableManager::* InFunc)(const FText&))
{
    return SAssignNew(InWidgetPtr, SEditableTextBox)
        .HintText(FText::FromString(HintTextValue))
        .Text(FText::FromString(TextBlockValue))
        .IsReadOnly(true)
        .IsEnabled(true)
        .MinDesiredWidth(EditableTextBox_Length)
        .ForegroundColor(FSlateColor(FLinearColor::White))
        .OnTextChanged(this, InFunc)
        .Justification(ETextJustify::Center);
}

TSharedRef<SWidget> SDataTableManager::CreateFolderBrowserButton(FReply(SDataTableManager::* InFunc)())
{
    return SNew(SButton)
        .ButtonStyle(FAppStyle::Get(), "FlatButton")
        .OnClicked(this, InFunc)
        .ContentPadding(FMargin(6, 2))
        [
            SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .VAlign(VAlign_Center)
                .AutoWidth()
                [
                    SNew(STextBlock)
                        .TextStyle(FAppStyle::Get(), "ContentBrowser.TopBar.Font")
                        .Font(FAppStyle::Get().GetFontStyle("FontAwesome.11"))
                        .Text(FEditorFontGlyphs::Folder)
                ]
        ];
}

FReply SDataTableManager::ExcelFolderBrowserBtnClicked()
{
    if (ExcelFolderPathWidget.IsValid() == false)
    {
        return FReply::Handled();
    }

    FString Result = OpenBrowserAndGetPath(ExcelFolderPathWidget->GetText().IsEmpty() ? FPaths::ProjectDir() : ExcelFolderPathWidget->GetText().ToString());

    if (Result.IsEmpty() == false)
    {
        ExcelFolderPathWidget->SetText(FText::FromString(Result));
    }

    return FReply::Handled();
}

FReply SDataTableManager::CSVFolderBrowserBtnClicked()
{
    if (CSVFolderPathWidget.IsValid() == false)
    {
        return FReply::Handled();
    }

    FString Result = OpenBrowserAndGetPath(CSVFolderPathWidget->GetText().IsEmpty() ? FPaths::ProjectDir() : CSVFolderPathWidget->GetText().ToString());

    if (Result.IsEmpty() == false)
    {
        CSVFolderPathWidget->SetText(FText::FromString(Result));
    }

    return FReply::Handled();
}

FReply SDataTableManager::StructFolderBrowserBtnClicked()
{
    if (StructFolderPathWidget.IsValid() == false)
    {
        return FReply::Handled();
    }

    FString Result = OpenBrowserAndGetPath(StructFolderPathWidget->GetText().IsEmpty() ? FPaths::GameSourceDir() : StructFolderPathWidget->GetText().ToString());

    if (Result.IsEmpty() == false)
    {
        StructFolderPathWidget->SetText(FText::FromString(Result));
    }

    return FReply::Handled();
}

FReply SDataTableManager::AssetFolderBrowserBtnClicked()
{
    if (AssetFolderPathWidget.IsValid() == false)
    {
        return FReply::Handled();
    }

    FString Result = OpenBrowserAndGetPath(AssetFolderPathWidget->GetText().IsEmpty() ? FPaths::ProjectContentDir() : AssetFolderPathWidget->GetText().ToString());

    if (Result.IsEmpty() == false)
    {
        AssetFolderPathWidget->SetText(FText::FromString(Result));
    }

    return FReply::Handled();
}

FString SDataTableManager::OpenBrowserAndGetPath(const FString& InPath)
{
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (!DesktopPlatform)
    {
        UE_LOG(LogTemp, Error, TEXT("Can't Use DesktopPlatformModule"));
        return "";
    }

    const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);

    FString OutFolderName;
    const bool bFileOpened = DesktopPlatform->OpenDirectoryDialog(
        ParentWindowHandle,
        TEXT("Select Folder"),
        InPath,
        OutFolderName
    );

    FString RelavtiveOutFolderName = FPaths::ConvertRelativePathToFull(OutFolderName);
    FString RelativeProjectDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
    FPaths::NormalizeDirectoryName(RelavtiveOutFolderName);
    FPaths::NormalizeDirectoryName(RelativeProjectDir);

    if (RelavtiveOutFolderName.StartsWith(RelativeProjectDir, ESearchCase::CaseSensitive) == false)
    {
        FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, LOCTEXT("ErrorMSG_SelectFolderPath", "Selected Folder Path is not in project directory"));
        return "";
    }

    return RelavtiveOutFolderName;
}

void SDataTableManager::OnCSVFolderPathTextChanged(const FText& NewText)
{
    TryCacheCSVFiles();
    RefreshCSVState();
    SaveConfig();
}

void SDataTableManager::OnExcelFolderPathTextChanged(const FText& NewText)
{
    TryCacheExcelFiles();
    TryCacheCSVFiles();
    RefreshSheetState();
    RefreshCSVState();
    SaveConfig();
}

void SDataTableManager::OnStructFolderPathTextChanged(const FText& NewText)
{
    SaveConfig();
}

void SDataTableManager::OnAssetFolderPathTextChanged(const FText& NewText)
{
    SaveConfig();
}

void SDataTableManager::LoadConfig()
{
    if (ToolConfig == nullptr)
    {
        return;
    }

    ToolConfig->LoadConfig();

    if (ExcelFolderPathWidget.IsValid())
    {
        ExcelFolderPathWidget->SetText(FText::FromString(ToolConfig->CachedExcelPath));
    }
    if (CSVFolderPathWidget.IsValid())
    {
        CSVFolderPathWidget->SetText(FText::FromString(ToolConfig->CachedCSVPath));
    }
    if (StructFolderPathWidget.IsValid())
    {
        StructFolderPathWidget->SetText(FText::FromString(ToolConfig->CachedStructPath));
    }
    if (AssetFolderPathWidget.IsValid())
    {
        AssetFolderPathWidget->SetText(FText::FromString(ToolConfig->CachedAssetPath));
    }
}

void SDataTableManager::SaveConfig()
{
    if (ToolConfig == nullptr || ExcelFolderPathWidget.IsValid() == false || CSVFolderPathWidget.IsValid() == false || StructFolderPathWidget.IsValid() == false || AssetFolderPathWidget.IsValid() == false)
    {
        return;
    }

    ToolConfig->CachedExcelPath = ExcelFolderPathWidget->GetText().ToString();
    ToolConfig->CachedCSVPath = CSVFolderPathWidget->GetText().ToString();
    ToolConfig->CachedStructPath = StructFolderPathWidget->GetText().ToString();
    ToolConfig->CachedAssetPath = AssetFolderPathWidget->GetText().ToString();

    ToolConfig->SaveConfig();
}


void SDataTableManager::RefreshSheetState()
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
            FString* CSVFullPath = CSVFiles.FindByPredicate(FindBySheetName_Functer(SheetName));
            TWeakObjectPtr<UScriptStruct>* UStructObj = UStructObjs.FindByPredicate(FindBySheetName_Functer(SheetName));

            DataList.Add(MakeShared<FSheetListRowData>(SheetName, ExcelFile, CSVFullPath == nullptr ? "" : *CSVFullPath, UStructObj == nullptr ? nullptr : (*UStructObj).Get(), false));
        }
    }

    ListView->RequestListRefresh();
}


void SDataTableManager::RefreshCSVState()
{
    for (TSharedPtr<FSheetListRowData> Data : DataList)
    {
        if (Data.IsValid())
        {
            FString* CSVFullPath = CSVFiles.FindByPredicate(FindBySheetName_Functer(Data->GetSheetName()));
            if (CSVFullPath != nullptr)
            {
                Data->SetCSVFullPath(*CSVFullPath);
            }
        }
    }
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
    if (ToolConfig == nullptr || ToolConfig->CachedCSVPath.IsEmpty())
    {
        return;
    }

    CSVFiles.Empty();

    XlsxManager::FindAllFilesInFolderPath(CSVFiles, ToolConfig->CachedCSVPath, ".csv");
}

void SDataTableManager::TryCacheExcelFiles()
{
    if (ToolConfig == nullptr || ToolConfig->CachedExcelPath.IsEmpty())
    {
        return;
    }

    ExcelFiles.Empty();

    XlsxManager::FindAllFilesInFolderPath(ExcelFiles, ToolConfig->CachedExcelPath, ".xlsx");
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
    FString CSVFolderPath = CSVFolderPathWidget->GetText().ToString();

    if (CSVFolderPath.IsEmpty() || FPaths::DirectoryExists(CSVFolderPath) == false)
    {
        FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, LOCTEXT("ErrorMSG_FolderPath", "CSV Folder Path is not exist or invalid"));
        return FReply::Handled();
    }

    TMap<FString,TArray<FString>> SheetMap;

    for (TSharedPtr<FSheetListRowData> Data : DataList)
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

    RefreshCSVState();

    FMessageDialog::Open(EAppMsgCategory::Success, EAppMsgType::Ok, LOCTEXT("SuccessMSG_ConvertCSV", "Convert CSV Success"));

    return FReply::Handled();
}

FReply SDataTableManager::OnStructGeneratorClicked()
{
    FString CSVFolderPath = CSVFolderPathWidget->GetText().ToString();
    FString StructFolderPath = StructFolderPathWidget->GetText().ToString();

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

    for (TSharedPtr<FSheetListRowData> Data : DataList)
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
    FString AssetFolderPath = AssetFolderPathWidget->GetText().ToString();

    if (AssetFolderPath.IsEmpty() || FPaths::DirectoryExists(AssetFolderPath) == false)
    {
        FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, LOCTEXT("ErrorMSG_FolderPath", "Asset Folder Path is not exist or invalid"));
        return FReply::Handled();
    }

    bool Result = false;

    for (TSharedPtr<FSheetListRowData> Data : DataList)
    {
        if (Data.IsValid() && Data->IsChecked())
        {
            Result = DataTableAssetGanerator::CreateDataTableFromCSV(Data->GetSheetName(), Data->GetCSVFullPath(), ToolConfig->CachedAssetPath, Data->GetStructure());

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
        FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, LOCTEXT("SuccessMSG_CreateAsset", "Create Data Table Failed"));
    }

    return FReply::Handled();
}

void SDataTableManager::OnCheckStateChanged(ECheckBoxState NewState)
{
    for (TSharedPtr<FSheetListRowData> Data : DataList)
    {
        if (Data.IsValid())
        {
            Data->SetIsChecked(NewState == ECheckBoxState::Checked);
        }
    }
}

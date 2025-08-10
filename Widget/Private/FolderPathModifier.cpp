#include "FolderPathModifier.h"

#include "Editor.h"
#include "EditorStyleSet.h"
#include "EditorFontGlyphs.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "DesktopPlatformModule.h"
#include "Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "DataTableManager"

static FText ErrorMSG_SelectFolderPath = LOCTEXT("ErrorMSG_SelectFolderPath", "Selected Folder Path is not in project directory");
static FText ErrorMSG_DesktopPlatformModule = LOCTEXT("ErrorMSG_DesktopPlatformModule", "Can't use DesktopPlatformModule");
static FText ErrorMSG_WindowHandle = LOCTEXT("ErrorMSG_WindowHandle", "Can't find WindowHandle");

static const float EditableTextBox_Length = 1125.f;
static const float TextBlock_Length = 100.f;

void SFolderPathModifier::Construct(const FArguments& InArgs)
{
    ToolConfig = GetMutableDefault<UDataTableManagerConfig>();
    ToolConfig->LoadConfig();

    PathType = InArgs._PathType;
    Title_Str = InArgs._Title_Str;
    Hint_Str = InArgs._Hint_Str;
    DefaultPath = InArgs._DefaultPath;

    TextChangedDelegate = InArgs._OnTextChanged;
    ButtonClickedDelegate = InArgs._OnButtonClicked;


    this->ChildSlot
        [
            SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Left)
                .Padding(6, 0)
                [
                    CreateFolderPathTitle()
                ]
                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
                .MaxWidth(EditableTextBox_Length)
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Left)
                .Padding(6, 0)
                [
                    CreateFolderPathTextBox()
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Left)
                .Padding(6, 0)
                [
                    CreateFolderBrowserButton()
                ]
        ];
}

TSharedRef<SWidget> SFolderPathModifier::CreateFolderPathTitle()
{
    return SAssignNew(Title_TextBlock, STextBlock)
        .Text(FText::FromString(Title_Str))
        .MinDesiredWidth(TextBlock_Length)
        .ColorAndOpacity(FSlateColor(FLinearColor::White));
}

TSharedRef<SWidget> SFolderPathModifier::CreateFolderPathTextBox()
{
    return SAssignNew(FolderPath_TextBox, SEditableTextBox)
        .HintText(FText::FromString(Hint_Str))
        .Text(FText::FromString(*GetPathPtrOnType()))
        .IsReadOnly(true)
        .IsEnabled(true)
        .MinDesiredWidth(EditableTextBox_Length)
        .ForegroundColor(FSlateColor(FLinearColor::White))
        .OnTextChanged(this, &SFolderPathModifier::OnFolderPathTextChanged)
        .Justification(ETextJustify::Center);
}

TSharedRef<SWidget> SFolderPathModifier::CreateFolderBrowserButton()
{
    return SAssignNew(OpenBrowser_Button, SButton)
		.ButtonStyle(FAppStyle::Get(), "FlatButton")
		.OnClicked(this, &SFolderPathModifier::FolderBrowserBtnClicked)
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

FReply SFolderPathModifier::FolderBrowserBtnClicked()
{
    if (FolderPath_TextBox.IsValid() == false)
    {
        return FReply::Handled();
    }

    FString Result = OpenBrowserAndGetPath(FolderPath_TextBox->GetText().IsEmpty() ? DefaultPath : FolderPath_TextBox->GetText().ToString());

    if (Result.IsEmpty() == false)
    {
        FolderPath_TextBox->SetText(FText::FromString(Result));
    }

    if (ButtonClickedDelegate.IsBound())
    {
        ButtonClickedDelegate.Execute();
    }

    return FReply::Handled();
}

const FString SFolderPathModifier::OpenBrowserAndGetPath(const FString& InPath)
{
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (DesktopPlatform == nullptr)
    {
        FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, ErrorMSG_DesktopPlatformModule);
        return "";
    }

    // 에디터 위에 띄우기 위해 최상위 Dialog 가져오기
    const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
    if (ParentWindowHandle == nullptr)
    {
        FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, ErrorMSG_WindowHandle);
        return "";
    }

    FString OutFolderName;

    // 폴더 브라우저 오픈
    const bool bFileOpened = DesktopPlatform->OpenDirectoryDialog(
        ParentWindowHandle,
        TEXT("Select Folder"),
        InPath,
        OutFolderName
    );

    // 선택한 폴더 경로가 프로젝트 경로에 포함되는지 확인
    FString RelavtiveOutFolderName = FPaths::ConvertRelativePathToFull(OutFolderName);
    FString RelativeProjectDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
    FPaths::NormalizeDirectoryName(RelavtiveOutFolderName);
    FPaths::NormalizeDirectoryName(RelativeProjectDir);

    if (RelavtiveOutFolderName.StartsWith(RelativeProjectDir, ESearchCase::CaseSensitive) == false)
    {
        FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, ErrorMSG_SelectFolderPath);
        return "";
    }

    return RelavtiveOutFolderName;
}

void SFolderPathModifier::OnFolderPathTextChanged(const FText& NewText)
{
    FString* CachedData = GetPathPtrOnType();

    if (CachedData != nullptr)
    {
        *CachedData = NewText.ToString();
    }

    ToolConfig->SaveConfig();

    TextChangedDelegate.ExecuteIfBound(NewText);
}

FString* SFolderPathModifier::GetPathPtrOnType()
{
    if (ToolConfig == nullptr)
    {
        return nullptr;
    }

    FString* CachedData = nullptr;

    switch (PathType)
    {
    case NONE:
        break;
    case PATH_EXCEL:
        CachedData = &ToolConfig->CachedExcelPath;
        break;
    case PATH_CSV:
        CachedData = &ToolConfig->CachedCSVPath;
        break;
    case PATH_STRUCT:
        CachedData = &ToolConfig->CachedStructPath;
        break;
    case PATH_ASSET:
        CachedData = &ToolConfig->CachedAssetPath;
        break;
    default:
        break;
    }

    return CachedData;
}

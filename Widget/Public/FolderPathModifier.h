#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "DataTableManagerConfig.h"

class DATATABLEMODULE_API SFolderPathModifier : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SFolderPathModifier){}

    SLATE_ARGUMENT(FString, Title_Str)
    SLATE_ARGUMENT(FString, Hint_Str)
    SLATE_ARGUMENT(FString, DefaultPath)
    SLATE_ARGUMENT(EPathType, PathType)
    SLATE_EVENT(FOnTextChanged, OnTextChanged)
    SLATE_EVENT(FOnClicked, OnButtonClicked)

    SLATE_END_ARGS()
public:

    void Construct(const FArguments& InArgs);
    FString* GetPathPtrOnType();

private:
    TSharedRef<SWidget> CreateFolderPathTitle();
    TSharedRef<SWidget> CreateFolderPathTextBox();
    TSharedRef<SWidget> CreateFolderBrowserButton();

private:
    FReply FolderBrowserBtnClicked();
    void OnFolderPathTextChanged(const FText& NewText);

    const FString OpenBrowserAndGetPath(const FString& InPath);

private:
    //Slate
    TSharedPtr<STextBlock> Title_TextBlock = nullptr;
    TSharedPtr<SEditableTextBox> FolderPath_TextBox = nullptr;
    TSharedPtr<SButton> OpenBrowser_Button = nullptr;

    EPathType PathType = EPathType::NONE;
    FString Title_Str = "";
    FString Hint_Str = "";
    FString DefaultPath = "";


    FOnTextChanged TextChangedDelegate;
    FOnClicked ButtonClickedDelegate;

    TObjectPtr<UDataTableManagerConfig> ToolConfig;
};

#include "DataTableModule.h"
#include "Modules/ModuleManager.h"
#include "LevelEditor.h"
#include "Textures/SlateIcon.h"

#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "ToolMenus.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Editor/EditorEngine.h"
#include "DataTableManager.h"
#include "EditorStyleSet.h"

static const FName DataTableManagerTabName("DataTableManager");

DEFINE_LOG_CATEGORY(DataTableModule);

#define LOCTEXT_NAMESPACE "FDataTableModule"

void FDataTableModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(DataTableManagerTabName, FOnSpawnTab::CreateRaw(this, &FDataTableModule::OnSpawnedTab))
		.SetDisplayName(LOCTEXT("FDataTableManagerTabTitle", "Data Table Manager"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	RegisterMenus();
}

void FDataTableModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(DataTableManagerTabName);
}


void FDataTableModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
	FToolMenuSection& Section = Menu->FindOrAddSection("CustomTools");
	Section.AddMenuEntry(
		"OpenDataTableManager",
		LOCTEXT("DataTableManagerMenuEntry", "Data Table Manager"),
		LOCTEXT("DataTableManagerMenuEntryTooltip", "Manage Xlsx, CSV, DataTable Assets"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(),"ClassIcon.DataTable"),
		FUIAction(FExecuteAction::CreateLambda([this]() {
			FGlobalTabmanager::Get()->TryInvokeTab(DataTableManagerTabName);
			}))
	);
}

TSharedRef<SDockTab> FDataTableModule::OnSpawnedTab(const class FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SAssignNew(DataTableManager,SDataTableManager)
				]
		];;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDataTableModule, DataTableModule)
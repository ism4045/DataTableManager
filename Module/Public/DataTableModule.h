#pragma once

#include "Modules/ModuleManager.h"
#include "DataTableManager.h"

DECLARE_LOG_CATEGORY_EXTERN(DataTableModule, All, All);

class DATATABLEMODULE_API FDataTableModule : public IModuleInterface
{
public:

	/* Called when the module is loaded */
	virtual void StartupModule() override;

	/* Called when the module is unloaded */
	virtual void ShutdownModule() override;

private:
	void RegisterMenus();

	TSharedRef<SDockTab> OnSpawnedTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	TSharedPtr<SDataTableManager> DataTableManager;
};
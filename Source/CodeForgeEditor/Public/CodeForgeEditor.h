#pragma once

#include "Modules/ModuleManager.h"

class FCodeForgeAssetTypeActions;
class FCodeForgeNodeFactory;

class FCodeForgeEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedPtr<FCodeForgeAssetTypeActions> CreatedAssetTypeActions;
	TSharedPtr<FCodeForgeNodeFactory> NodeFactory;

	void RegisterToolbarButton();
	static void CreateNewCodeForgeBlueprint();
};



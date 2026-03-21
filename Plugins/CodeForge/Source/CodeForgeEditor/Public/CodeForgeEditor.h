#pragma once

#include "Modules/ModuleManager.h"

class FCodeForgeAssetTypeActions;

class FCodeForgeEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedPtr<FCodeForgeAssetTypeActions> CreatedAssetTypeActions;
};

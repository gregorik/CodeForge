// Copyright (c) 2026 GregOrigin. All Rights Reserved.
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

	/** UClass FNames we registered IDetailCustomization for (so we can unregister cleanly). */
	TArray<FName> RegisteredDetailCustomizationClasses;

	void RegisterToolbarButton();
	void RegisterDetailCustomizations();
	void UnregisterDetailCustomizations();
	static void CreateNewCodeForgeBlueprint();
};

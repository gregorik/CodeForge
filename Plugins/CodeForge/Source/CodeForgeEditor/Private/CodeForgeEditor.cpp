#include "CodeForgeEditor.h"
#include "CodeForgeAssetTypeActions.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"

#define LOCTEXT_NAMESPACE "FCodeForgeEditorModule"

void FCodeForgeEditorModule::StartupModule()
{
	IAssetTools& AssetTools =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	CreatedAssetTypeActions = MakeShared<FCodeForgeAssetTypeActions>();
	AssetTools.RegisterAssetTypeActions(CreatedAssetTypeActions.ToSharedRef());
}

void FCodeForgeEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools") && CreatedAssetTypeActions.IsValid())
	{
		IAssetTools& AssetTools =
			FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions.ToSharedRef());
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCodeForgeEditorModule, CodeForgeEditor)

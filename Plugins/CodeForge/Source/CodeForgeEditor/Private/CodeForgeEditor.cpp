#include "CodeForgeEditor.h"
#include "CodeForgeAssetTypeActions.h"
#include "CodeForgeNodeFactory.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "EdGraphUtilities.h"

#define LOCTEXT_NAMESPACE "FCodeForgeEditorModule"

void FCodeForgeEditorModule::StartupModule()
{
	IAssetTools& AssetTools =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	CreatedAssetTypeActions = MakeShared<FCodeForgeAssetTypeActions>();
	AssetTools.RegisterAssetTypeActions(CreatedAssetTypeActions.ToSharedRef());

	NodeFactory = MakeShared<FCodeForgeNodeFactory>();
	FEdGraphUtilities::RegisterVisualNodeFactory(NodeFactory);
}

void FCodeForgeEditorModule::ShutdownModule()
{
	if (NodeFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(NodeFactory);
		NodeFactory.Reset();
	}

	if (FModuleManager::Get().IsModuleLoaded("AssetTools") && CreatedAssetTypeActions.IsValid())
	{
		IAssetTools& AssetTools =
			FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions.ToSharedRef());
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCodeForgeEditorModule, CodeForgeEditor)

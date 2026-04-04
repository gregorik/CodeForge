// Copyright (c) 2026 GregOrigin. All Rights Reserved.

#include "CodeForgeEditor.h"
#include "CodeForgeAssetTypeActions.h"
#include "CodeForgeAssetFactory.h"
#include "CodeForgeNodeFactory.h"
#include "CodeForgeThumbnailRenderer.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgeSettings.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "EdGraphUtilities.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FCodeForgeEditorModule"

void FCodeForgeEditorModule::StartupModule()
{
	IAssetTools& AssetTools =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	CreatedAssetTypeActions = MakeShared<FCodeForgeAssetTypeActions>();
	AssetTools.RegisterAssetTypeActions(CreatedAssetTypeActions.ToSharedRef());

	NodeFactory = MakeShared<FCodeForgeNodeFactory>();
	FEdGraphUtilities::RegisterVisualNodeFactory(NodeFactory);

	UThumbnailManager::Get().RegisterCustomRenderer(
		UCodeForgeBlueprint::StaticClass(),
		UCodeForgeThumbnailRenderer::StaticClass());

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FCodeForgeEditorModule::RegisterToolbarButton));
}

void FCodeForgeEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	if (NodeFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(NodeFactory);
		NodeFactory.Reset();
	}

	if (UObjectInitialized())
	{
		UThumbnailManager::Get().UnregisterCustomRenderer(UCodeForgeBlueprint::StaticClass());
	}

	if (FModuleManager::Get().IsModuleLoaded("AssetTools") && CreatedAssetTypeActions.IsValid())
	{
		IAssetTools& AssetTools =
			FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions.ToSharedRef());
	}
}

// ---------------------------------------------------------------------------
// Toolbar button
// ---------------------------------------------------------------------------

void FCodeForgeEditorModule::RegisterToolbarButton()
{
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
	if (!ToolbarMenu)
	{
		return;
	}

	FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("CodeForge");

	FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(
		"CreateCodeForgeBlueprint",
		FToolUIActionChoice(FExecuteAction::CreateStatic(&FCodeForgeEditorModule::CreateNewCodeForgeBlueprint)),
		LOCTEXT("CreateCFBlueprint", "Create CodeForge Blueprint"),
		LOCTEXT("CreateCFBlueprintTooltip", "Create a new CodeForge Blueprint asset and open it in the graph editor"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "MainFrame.AddCodeToProject")
	));

	// Visibility delegate so toggling the setting takes effect immediately
	Entry.Visibility = TAttribute<bool>::CreateLambda([]() -> bool
	{
		return GetDefault<UCodeForgeSettings>()->bShowToolbarButton;
	});
}

void FCodeForgeEditorModule::CreateNewCodeForgeBlueprint()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	UCodeForgeAssetFactory* Factory = NewObject<UCodeForgeAssetFactory>();

	// CreateAssetWithDialog lets the engine handle unique naming and opens the editor (bEditAfterNew)
	AssetTools.CreateAssetWithDialog(UCodeForgeBlueprint::StaticClass(), Factory);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCodeForgeEditorModule, CodeForgeEditor)




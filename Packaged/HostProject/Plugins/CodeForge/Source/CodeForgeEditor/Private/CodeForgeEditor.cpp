// Copyright (c) 2026 GregOrigin. All Rights Reserved.
#include "CodeForgeEditor.h"
#include "CodeForgeAssetEditor.h"
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
#include "CodeForgeStyle.h"
#include "PropertyEditorModule.h"
#include "Details/CodeForgeNode_PropertyDetails.h"
#include "Details/CodeForgeNode_FunctionDetails.h"
#include "Details/CodeForgeNode_ClassDetails.h"
#include "Details/CodeForgeNode_RPCDetails.h"
#include "Details/CodeForgeNode_StructDetails.h"
#include "Details/CodeForgeNode_EnumDetails.h"
#include "CodeForgeNode_Property.h"
#include "CodeForgeNode_Function.h"
#include "CodeForgeNode_Class.h"
#include "CodeForgeNode_RPC.h"
#include "CodeForgeNode_Struct.h"
#include "CodeForgeNode_Enum.h"
#include "CodeForgeNode_Enum.h"
#include "Interfaces/IPluginManager.h"
#include "Interfaces/IMainFrameModule.h"
#include "Editor.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "FCodeForgeEditorModule"

void FCodeForgeEditorModule::StartupModule()
{
	FCodeForgeStyle::Initialize();

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

	RegisterDetailCustomizations();
}

void FCodeForgeEditorModule::ShutdownModule()
{
	UnregisterDetailCustomizations();

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

	FCodeForgeStyle::Shutdown();
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

	FUIAction ToolbarAction(FExecuteAction::CreateStatic(&FCodeForgeEditorModule::CreateNewCodeForgeBlueprint));
	ToolbarAction.IsActionVisibleDelegate = FIsActionButtonVisible::CreateLambda([]() -> bool
	{
		return GetDefault<UCodeForgeSettings>()->bShowToolbarButton;
	});

	Section.AddEntry(FToolMenuEntry::InitToolBarButton(
		"CreateCodeForgeBlueprint",
		FToolUIActionChoice(ToolbarAction),
		LOCTEXT("CreateCFBlueprint", "Create CodeForge Blueprint"),
		LOCTEXT("CreateCFBlueprintTooltip", "Create a new CodeForge Blueprint asset and open it in the graph editor"),
		TAttribute<FSlateIcon>::CreateLambda([]() -> FSlateIcon
		{
			if (GetDefault<UCodeForgeSettings>()->bUseCustomToolbarIcons)
			{
				return FSlateIcon(FCodeForgeStyle::GetStyleSetName(), "CodeForge.ToolbarIcon");
			}
			return FSlateIcon(FAppStyle::GetAppStyleSetName(), "MainFrame.AddCodeToProject");
		})
	));
}

void FCodeForgeEditorModule::CreateNewCodeForgeBlueprint()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	UCodeForgeAssetFactory* Factory = NewObject<UCodeForgeAssetFactory>();

	// CreateAssetWithDialog lets the engine handle unique naming and opens the editor (bEditAfterNew)
	AssetTools.CreateAssetWithDialog(UCodeForgeBlueprint::StaticClass(), Factory);
}

// ---------------------------------------------------------------------------
// Detail customizations (Common/Advanced split for graph node properties)
// ---------------------------------------------------------------------------

void FCodeForgeEditorModule::RegisterDetailCustomizations()
{
	FPropertyEditorModule& PropertyModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	auto Register = [&](UClass* Class, FOnGetDetailCustomizationInstance Maker)
	{
		PropertyModule.RegisterCustomClassLayout(Class->GetFName(), Maker);
		RegisteredDetailCustomizationClasses.Add(Class->GetFName());
	};

	Register(UCodeForgeNode_Property::StaticClass(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FCodeForgeNode_PropertyDetails::MakeInstance));
	Register(UCodeForgeNode_Function::StaticClass(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FCodeForgeNode_FunctionDetails::MakeInstance));
	Register(UCodeForgeNode_Class::StaticClass(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FCodeForgeNode_ClassDetails::MakeInstance));
	Register(UCodeForgeNode_RPC::StaticClass(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FCodeForgeNode_RPCDetails::MakeInstance));
	Register(UCodeForgeNode_Struct::StaticClass(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FCodeForgeNode_StructDetails::MakeInstance));
	Register(UCodeForgeNode_Enum::StaticClass(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FCodeForgeNode_EnumDetails::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FCodeForgeEditorModule::UnregisterDetailCustomizations()
{
	if (!FModuleManager::Get().IsModuleLoaded("PropertyEditor")) return;

	FPropertyEditorModule& PropertyModule =
		FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	for (const FName& ClassName : RegisteredDetailCustomizationClasses)
	{
		PropertyModule.UnregisterCustomClassLayout(ClassName);
	}
	RegisteredDetailCustomizationClasses.Reset();

	PropertyModule.NotifyCustomizationModuleChanged();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCodeForgeEditorModule, CodeForgeEditor)
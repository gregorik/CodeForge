// CodeForgeAssetEditor.cpp
// Implements the graph-based asset editor for UCodeForgeBlueprint.

#include "CodeForgeAssetEditor.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgeEdGraph.h"
#include "CodeForgeEdGraphSchema.h"
#include "CodeForgeGenerator.h"

#include "GraphEditor.h"
#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "ScopedTransaction.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Commands/GenericCommands.h"
#include "GraphEditorActions.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "CodeForgeAssetEditor"

// ---------------------------------------------------------------------------
// Static tab identifiers
// ---------------------------------------------------------------------------

const FName FCodeForgeAssetEditor::GraphTabId(TEXT("CodeForgeGraphTab"));
const FName FCodeForgeAssetEditor::DetailsTabId(TEXT("CodeForgeDetailsTab"));
const FName FCodeForgeAssetEditor::PreviewTabId(TEXT("CodeForgePreviewTab"));

// ---------------------------------------------------------------------------
// InitEditor
// ---------------------------------------------------------------------------

void FCodeForgeAssetEditor::InitEditor(
	const EToolkitMode::Type Mode,
	const TSharedPtr<IToolkitHost>& InitToolkitHost,
	UCodeForgeBlueprint* InBlueprint)
{
	Blueprint = InBlueprint;
	check(Blueprint);

	CreateEdGraphIfNeeded();
	EdGraph->SyncAllNodesFromBlueprint();

	// Set up graph editor commands
	GraphEditorCommands = MakeShared<FUICommandList>();
	GraphEditorCommands->MapAction(
		FGenericCommands::Get().Delete,
		FExecuteAction::CreateLambda([this]()
		{
			if (GraphEditorWidget.IsValid())
			{
				const FScopedTransaction Transaction(FText::FromString(TEXT("Delete CodeForge Node")));
				GraphEditorWidget->GetCurrentGraph()->Modify();
				const FGraphPanelSelectionSet SelectedNodes = GraphEditorWidget->GetSelectedNodes();
				for (UObject* NodeObj : SelectedNodes)
				{
					if (UEdGraphNode* Node = Cast<UEdGraphNode>(NodeObj))
					{
						if (Node->CanUserDeleteNode())
						{
							Node->DestroyNode();
						}
					}
				}
			}
		}),
		FCanExecuteAction::CreateLambda([]() { return true; })
	);

	// Define the editor layout: graph (50%), details (25%), preview (25%)
	const TSharedRef<FTabManager::FLayout> Layout =
		FTabManager::NewLayout("CodeForgeAssetEditorLayout_v2")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.5f)
				->AddTab(GraphTabId, ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.25f)
				->AddTab(DetailsTabId, ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.25f)
				->AddTab(PreviewTabId, ETabState::OpenedTab)
			)
		);

	// Add a toolbar extender for the Generate button
	TSharedPtr<FExtender> ToolbarExtender = MakeShared<FExtender>();
	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		nullptr,
		FToolBarExtensionDelegate::CreateLambda(
			[this](FToolBarBuilder& Builder)
			{
				Builder.AddToolBarButton(
					FUIAction(FExecuteAction::CreateRaw(this, &FCodeForgeAssetEditor::OnGenerateClicked)),
					NAME_None,
					LOCTEXT("GenerateButton", "Generate"),
					LOCTEXT("GenerateTooltip", "Generate C++ code from this CodeForge Blueprint"),
					FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Recompile")
				);
			}
		)
	);
	AddToolbarExtender(ToolbarExtender);

	// Initialize the asset editor toolkit
	InitAssetEditor(
		Mode,
		InitToolkitHost,
		TEXT("CodeForgeAssetEditorApp"),
		Layout,
		/*bCreateDefaultStandaloneMenu=*/ true,
		/*bCreateDefaultToolbar=*/ true,
		InBlueprint);

	RegenerateMenusAndToolbars();
	RefreshPreview();
}

// ---------------------------------------------------------------------------
// CreateEdGraphIfNeeded
// ---------------------------------------------------------------------------

void FCodeForgeAssetEditor::CreateEdGraphIfNeeded()
{
	// Check if the blueprint already has a graph subobject
	ForEachObjectWithOuter(Blueprint, [this](UObject* Obj)
	{
		if (UCodeForgeEdGraph* ExistingGraph = Cast<UCodeForgeEdGraph>(Obj))
		{
			EdGraph = ExistingGraph;
		}
	});

	if (EdGraph)
	{
		EdGraph->CodeForgeBlueprint = Blueprint;
		return;
	}

	// No existing graph found -- create a new one as a subobject of the blueprint
	EdGraph = NewObject<UCodeForgeEdGraph>(Blueprint, NAME_None, RF_Transactional);
	EdGraph->Schema = UCodeForgeEdGraphSchema::StaticClass();
	EdGraph->CodeForgeBlueprint = Blueprint;
	Blueprint->MarkPackageDirty();
}

// ---------------------------------------------------------------------------
// FAssetEditorToolkit interface
// ---------------------------------------------------------------------------

FName FCodeForgeAssetEditor::GetToolkitFName() const
{
	return FName(TEXT("CodeForgeAssetEditor"));
}

FText FCodeForgeAssetEditor::GetBaseToolkitName() const
{
	return LOCTEXT("ToolkitName", "CodeForge Editor");
}

FString FCodeForgeAssetEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("CodeForge ");
}

FLinearColor FCodeForgeAssetEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.0f, 0.47f, 0.84f, 1.0f);
}

// ---------------------------------------------------------------------------
// Tab spawners
// ---------------------------------------------------------------------------

void FCodeForgeAssetEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(
		LOCTEXT("WorkspaceMenu", "CodeForge Editor"));

	InTabManager->RegisterTabSpawner(
		GraphTabId,
		FOnSpawnTab::CreateRaw(this, &FCodeForgeAssetEditor::SpawnGraphTab))
		.SetDisplayName(LOCTEXT("GraphTab", "Graph"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef());

	InTabManager->RegisterTabSpawner(
		DetailsTabId,
		FOnSpawnTab::CreateRaw(this, &FCodeForgeAssetEditor::SpawnDetailsTab))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef());

	InTabManager->RegisterTabSpawner(
		PreviewTabId,
		FOnSpawnTab::CreateRaw(this, &FCodeForgeAssetEditor::SpawnPreviewTab))
		.SetDisplayName(LOCTEXT("PreviewTab", "Preview"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

void FCodeForgeAssetEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	InTabManager->UnregisterTabSpawner(GraphTabId);
	InTabManager->UnregisterTabSpawner(DetailsTabId);
	InTabManager->UnregisterTabSpawner(PreviewTabId);

	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
}

// ---------------------------------------------------------------------------
// SpawnGraphTab
// ---------------------------------------------------------------------------

TSharedRef<SDockTab> FCodeForgeAssetEditor::SpawnGraphTab(const FSpawnTabArgs& Args)
{
	SGraphEditor::FGraphEditorEvents Events;
	Events.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateRaw(
		this, &FCodeForgeAssetEditor::OnGraphSelectionChanged);

	GraphEditorWidget = SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.GraphToEdit(EdGraph)
		.GraphEvents(Events)
		.IsEditable(true);

	return SNew(SDockTab)
		.Label(LOCTEXT("GraphTabLabel", "Graph"))
		.TabColorScale(GetTabColorScale())
		[
			GraphEditorWidget.ToSharedRef()
		];
}

// ---------------------------------------------------------------------------
// SpawnDetailsTab
// ---------------------------------------------------------------------------

TSharedRef<SDockTab> FCodeForgeAssetEditor::SpawnDetailsTab(const FSpawnTabArgs& Args)
{
	FPropertyEditorModule& PropertyEditorModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.bHideSelectionTip = true;

	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	// Show the blueprint properties by default
	if (Blueprint)
	{
		DetailsView->SetObject(Blueprint);
	}

	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTabLabel", "Details"))
		.TabColorScale(GetTabColorScale())
		[
			DetailsView.ToSharedRef()
		];
}

// ---------------------------------------------------------------------------
// SpawnPreviewTab
// ---------------------------------------------------------------------------

TSharedRef<SDockTab> FCodeForgeAssetEditor::SpawnPreviewTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("PreviewTabLabel", "Code Preview"))
		.TabColorScale(GetTabColorScale())
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("HeaderLabel", "Header (.h)"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
			]

			+ SVerticalBox::Slot()
			.FillHeight(0.5f)
			.Padding(4.f)
			[
				SAssignNew(PreviewHeaderText, SMultiLineEditableTextBox)
				.IsReadOnly(true)
				.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SourceLabel", "Source (.cpp)"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
			]

			+ SVerticalBox::Slot()
			.FillHeight(0.5f)
			.Padding(4.f)
			[
				SAssignNew(PreviewSourceText, SMultiLineEditableTextBox)
				.IsReadOnly(true)
				.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
			]
		];
}

// ---------------------------------------------------------------------------
// RefreshPreview
// ---------------------------------------------------------------------------

void FCodeForgeAssetEditor::RefreshPreview()
{
	if (!Blueprint || !EdGraph) return;

	EdGraph->SyncAllNodesToBlueprint();

	FCodeForgeGenerator Generator;
	FCodeForgeGeneratorResult Result = Generator.Generate(Blueprint);

	if (PreviewHeaderText.IsValid())
	{
		PreviewHeaderText->SetText(Result.bSuccess
			? FText::FromString(Result.HeaderContent)
			: FText::FromString(FString::Printf(TEXT("// Error: %s"), *Result.ErrorMessage)));
	}

	if (PreviewSourceText.IsValid())
	{
		PreviewSourceText->SetText(Result.bSuccess
			? FText::FromString(Result.SourceContent)
			: FText::GetEmpty());
	}
}

// ---------------------------------------------------------------------------
// OnGraphSelectionChanged
// ---------------------------------------------------------------------------

void FCodeForgeAssetEditor::OnGraphSelectionChanged(const TSet<UObject*>& SelectedNodes)
{
	if (DetailsView.IsValid())
	{
		if (SelectedNodes.Num() == 1)
		{
			for (UObject* Node : SelectedNodes)
			{
				DetailsView->SetObject(Node);
			}
		}
		else if (SelectedNodes.Num() == 0 && Blueprint)
		{
			// Nothing selected -- show the blueprint itself
			DetailsView->SetObject(Blueprint);
		}
		else
		{
			DetailsView->SetObject(nullptr);
		}
	}

	RefreshPreview();
}

// ---------------------------------------------------------------------------
// OnGenerateClicked
// ---------------------------------------------------------------------------

void FCodeForgeAssetEditor::OnGenerateClicked()
{
	if (!Blueprint || !EdGraph)
	{
		return;
	}

	// 1. Sync graph nodes to the blueprint data
	{
		const FScopedTransaction Transaction(FText::FromString(TEXT("Sync Graph to Blueprint")));
		Blueprint->Modify();
		EdGraph->SyncAllNodesToBlueprint();
	}

	// 2. Run the code generator
	FCodeForgeGenerator Generator;
	FCodeForgeGeneratorResult Result = Generator.GenerateAndIntegrate(
		Blueprint, FPaths::ProjectDir());

	if (!Result.bSuccess)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::Format(
				LOCTEXT("GenerateError", "Code generation failed:\n{0}"),
				FText::FromString(Result.ErrorMessage)));
		return;
	}

	// 3. Handle structural vs behavioral changes
	if (Result.bIsStructuralChange)
	{
		EAppReturnType::Type UserResponse = FMessageDialog::Open(
			EAppMsgType::YesNo,
			LOCTEXT("StructuralChange",
				"Code generation succeeded with structural changes.\n"
				"A hot reload or editor restart is required.\n\n"
				"Would you like to attempt Live Coding now?"));

		if (UserResponse == EAppReturnType::Yes)
		{
			IModuleInterface* LiveCodingModule =
				FModuleManager::Get().GetModule(TEXT("LiveCoding"));
			if (LiveCodingModule && GEngine)
			{
				GEngine->Exec(nullptr, TEXT("LiveCoding.Compile"));
			}
			else
			{
				FMessageDialog::Open(
					EAppMsgType::Ok,
					LOCTEXT("NoLiveCoding",
						"Live Coding is not available. Please restart the editor to pick up structural changes."));
			}
		}
	}
	else
	{
		// Behavioral change only -- try Live Coding automatically
		IModuleInterface* LiveCodingModule =
			FModuleManager::Get().GetModule(TEXT("LiveCoding"));
		if (LiveCodingModule && GEngine)
		{
			GEngine->Exec(nullptr, TEXT("LiveCoding.Compile"));
		}

		FMessageDialog::Open(
			EAppMsgType::Ok,
			LOCTEXT("GenerateSuccess", "Code generation succeeded."));
	}
}

#undef LOCTEXT_NAMESPACE

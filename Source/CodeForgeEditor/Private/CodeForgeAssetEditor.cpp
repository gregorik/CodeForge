// Copyright (c) 2026 GregOrigin. All Rights Reserved.

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
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SOverlay.h"
#include "CodeForgeNode_Class.h"
#include "CodeForgeNode_Struct.h"
#include "CodeForgeNode_Enum.h"
#include "CodeForgeEdGraphNode.h"
#include "CodeForgeSettings.h"
#include "CodeForgePropertyDef.h"
#include "CodeForgeFunctionDef.h"
#include "CodeForgeEnumDef.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/App.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "ObjectTools.h"

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
	EdGraph->PopulateFromBlueprint();
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
		FTabManager::NewLayout("CodeForgeAssetEditorLayout_v3")
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
// SaveAsset_Execute â€” sync graph nodes to blueprint before saving
// ---------------------------------------------------------------------------

void FCodeForgeAssetEditor::SaveAsset_Execute()
{
	if (EdGraph)
	{
		EdGraph->SyncAllNodesToBlueprint();
	}
	FAssetEditorToolkit::SaveAsset_Execute();
}

// ---------------------------------------------------------------------------
// SpawnGraphTab
// ---------------------------------------------------------------------------

TSharedRef<SDockTab> FCodeForgeAssetEditor::SpawnGraphTab(const FSpawnTabArgs& Args)
{
	const UCodeForgeSettings* CFSettings = GetDefault<UCodeForgeSettings>();
	const FLinearColor BrandAccent = CFSettings->BrandAccentColor;
	const FLinearColor BrandSecondary = CFSettings->BrandSecondaryColor;

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
		.TabColorScale(BrandAccent)
		[
			SNew(SVerticalBox)

			// â”€â”€ Branded toolbar strip â”€â”€
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(BrandSecondary)
				.Padding(FMargin(8.f, 5.f))
				[
					SNew(SHorizontalBox)

					// Brand label
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(FMargin(0.f, 0.f, 8.f, 0.f))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("GraphBrandLabel", "CODEFORGE"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
						.ColorAndOpacity(BrandAccent)
					]

					// Separator
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(FMargin(0.f, 0.f, 8.f, 0.f))
					[
						SNew(SBox)
						.WidthOverride(1.f)
						.HeightOverride(16.f)
						[
							SNew(SBorder)
							.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
							.BorderBackgroundColor(BrandAccent * 0.3f)
						]
					]

					// Search box
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.VAlign(VAlign_Center)
					[
						SAssignNew(SearchBox, SSearchBox)
						.OnTextChanged(FOnTextChanged::CreateRaw(this, &FCodeForgeAssetEditor::OnSearchTextChanged))
						.HintText(LOCTEXT("SearchHint", "Search nodes..."))
					]

					// Auto layout button
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(6.f, 0.f, 0.f, 0.f))
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("AutoLayout", "Auto Layout"))
						.OnClicked(FOnClicked::CreateRaw(this, &FCodeForgeAssetEditor::OnAutoLayoutClicked))
					]
				]
			]

			// â”€â”€ Accent line â”€â”€
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(2.f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(BrandAccent)
				]
			]

			// â”€â”€ Graph editor with branded watermark â”€â”€
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SNew(SOverlay)

				// Layer 0: the graph editor
				+ SOverlay::Slot()
				[
					GraphEditorWidget.ToSharedRef()
				]

				// Layer 1: bottom-right watermark (non-interactive)
				+ SOverlay::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Bottom)
				.Padding(FMargin(0.f, 0.f, 20.f, 16.f))
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("WatermarkBrand", "CODEFORGE"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
						.ColorAndOpacity(BrandAccent * FLinearColor(1.f, 1.f, 1.f, 0.06f))
						.Visibility(EVisibility::HitTestInvisible)
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("WatermarkSub", "Visual C++ Blueprint Designer"))
						.Font(FCoreStyle::GetDefaultFontStyle("Italic", 9))
						.ColorAndOpacity(FLinearColor(0.5f, 0.6f, 0.7f, 0.04f))
						.Visibility(EVisibility::HitTestInvisible)
					]
				]

				// Layer 2: top-left version badge (non-interactive)
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.Padding(FMargin(12.f, 8.f, 0.f, 0.f))
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.25f))
					.Padding(FMargin(6.f, 2.f))
					.Visibility(EVisibility::HitTestInvisible)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("VersionBadge", "CF v1.0"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
						.ColorAndOpacity(BrandAccent * FLinearColor(1.f, 1.f, 1.f, 0.35f))
					]
				]
			]
		];
}

// ---------------------------------------------------------------------------
// SpawnDetailsTab
// ---------------------------------------------------------------------------

TSharedRef<SDockTab> FCodeForgeAssetEditor::SpawnDetailsTab(const FSpawnTabArgs& Args)
{
	const UCodeForgeSettings* CFSettings = GetDefault<UCodeForgeSettings>();
	const FLinearColor BrandAccent = CFSettings->BrandAccentColor;
	const FLinearColor BrandSecondary = CFSettings->BrandSecondaryColor;

	FPropertyEditorModule& PropertyEditorModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.bHideSelectionTip = true;

	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	if (Blueprint)
	{
		DetailsView->SetObject(Blueprint);
	}

	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTabLabel", "Details"))
		.TabColorScale(BrandAccent)
		[
			SNew(SVerticalBox)

			// â”€â”€ Header bar â”€â”€
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(BrandSecondary)
				.Padding(FMargin(10.f, 6.f))
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("DetailsHeaderTitle", "NODE PROPERTIES"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
						.ColorAndOpacity(BrandAccent)
					]

					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						SNullWidget::NullWidget
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text_Lambda([this]()
						{
							if (!Blueprint) return FText::GetEmpty();
							return FText::FromString(Blueprint->ClassName);
						})
						.Font(FCoreStyle::GetDefaultFontStyle("Italic", 9))
						.ColorAndOpacity(FLinearColor(0.5f, 0.6f, 0.7f))
					]
				]
			]

			// â”€â”€ Accent line â”€â”€
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(2.f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(BrandAccent)
				]
			]

			// â”€â”€ Details view â”€â”€
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				DetailsView.ToSharedRef()
			]
		];
}

// ---------------------------------------------------------------------------
// SpawnPreviewTab
// ---------------------------------------------------------------------------

TSharedRef<SDockTab> FCodeForgeAssetEditor::SpawnPreviewTab(const FSpawnTabArgs& Args)
{
	const UCodeForgeSettings* CFSettings = GetDefault<UCodeForgeSettings>();
	const FLinearColor BrandAccent = CFSettings->BrandAccentColor;
	const FLinearColor BrandSecondary = CFSettings->BrandSecondaryColor;
	const FLinearColor CodeBg(0.04f, 0.05f, 0.08f);

	// Section header builder
	auto MakeSectionHeader = [&](const FText& Label, const FText& FileExt) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
			.BorderBackgroundColor(BrandSecondary * 0.8f)
			.Padding(FMargin(10.f, 5.f))
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
					.ColorAndOpacity(BrandAccent)
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				[
					SNullWidget::NullWidget
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FileExt)
					.Font(FCoreStyle::GetDefaultFontStyle("Mono", 8))
					.ColorAndOpacity(FLinearColor(0.4f, 0.5f, 0.6f))
				]
			];
	};

	return SNew(SDockTab)
		.Label(LOCTEXT("PreviewTabLabel", "Code Preview"))
		.TabColorScale(BrandAccent)
		[
			SNew(SVerticalBox)

			// â”€â”€ Top header bar â”€â”€
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(BrandSecondary)
				.Padding(FMargin(10.f, 6.f))
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("PreviewHeaderTitle", "GENERATED CODE"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
						.ColorAndOpacity(BrandAccent)
					]

					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						SNullWidget::NullWidget
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("PreviewLiveLabel", "LIVE"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
						.ColorAndOpacity(FLinearColor(0.2f, 0.9f, 0.4f))
					]
				]
			]

			// â”€â”€ Accent line â”€â”€
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(2.f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(BrandAccent)
				]
			]

			// â”€â”€ Header (.h) section â”€â”€
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				MakeSectionHeader(
					LOCTEXT("HeaderSectionLabel", "HEADER"),
					LOCTEXT("HeaderExt", ".h"))
			]

			+ SVerticalBox::Slot()
			.FillHeight(0.5f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(CodeBg)
				.Padding(FMargin(4.f, 2.f))
				[
					SAssignNew(PreviewHeaderText, SMultiLineEditableTextBox)
					.IsReadOnly(true)
					.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
					.ForegroundColor(FLinearColor(0.78f, 0.85f, 0.95f))
				]
			]

			// â”€â”€ Divider â”€â”€
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(1.f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(BrandAccent * 0.25f)
				]
			]

			// â”€â”€ Source (.cpp) section â”€â”€
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				MakeSectionHeader(
					LOCTEXT("SourceSectionLabel", "SOURCE"),
					LOCTEXT("SourceExt", ".cpp"))
			]

			+ SVerticalBox::Slot()
			.FillHeight(0.5f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(CodeBg)
				.Padding(FMargin(4.f, 2.f))
				[
					SAssignNew(PreviewSourceText, SMultiLineEditableTextBox)
					.IsReadOnly(true)
					.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
					.ForegroundColor(FLinearColor(0.78f, 0.85f, 0.95f))
				]
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

	if (EdGraph)
	{
		EdGraph->RunValidationOnNodes();
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

// ---------------------------------------------------------------------------
// OnSearchTextChanged
// ---------------------------------------------------------------------------

void FCodeForgeAssetEditor::OnSearchTextChanged(const FText& NewText)
{
	FString SearchText = NewText.ToString();

	if (!EdGraph) return;

	for (UEdGraphNode* Node : EdGraph->Nodes)
	{
		UCodeForgeEdGraphNode* CFNode = Cast<UCodeForgeEdGraphNode>(Node);
		if (!CFNode) continue;

		FString Title = CFNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString();
		bool bMatch = SearchText.IsEmpty() || Title.Contains(SearchText, ESearchCase::IgnoreCase);

		CFNode->bCommentBubbleMakeVisible = !bMatch && !SearchText.IsEmpty();
	}

	if (GraphEditorWidget.IsValid())
	{
		GraphEditorWidget->NotifyGraphChanged();
	}
}

// ---------------------------------------------------------------------------
// OnAutoLayoutClicked
// ---------------------------------------------------------------------------

FReply FCodeForgeAssetEditor::OnAutoLayoutClicked()
{
	if (!EdGraph) return FReply::Handled();

	const FScopedTransaction Transaction(FText::FromString(TEXT("Auto Layout Nodes")));
	EdGraph->Modify();

	TArray<UEdGraphNode*> TypeNodes;
	TArray<UEdGraphNode*> MemberNodes;

	for (UEdGraphNode* Node : EdGraph->Nodes)
	{
		if (Node->IsA<UCodeForgeNode_Class>() || Node->IsA<UCodeForgeNode_Struct>()
			|| Node->IsA<UCodeForgeNode_Enum>())
		{
			TypeNodes.Add(Node);
		}
		else
		{
			MemberNodes.Add(Node);
		}
	}

	int32 YOffset = 0;
	const int32 NodeSpacingY = 200;
	const int32 ColumnSpacing = 400;

	for (UEdGraphNode* Node : TypeNodes)
	{
		Node->NodePosX = 0;
		Node->NodePosY = YOffset;
		YOffset += NodeSpacingY;
	}

	YOffset = 0;
	for (UEdGraphNode* Node : MemberNodes)
	{
		Node->NodePosX = ColumnSpacing;
		Node->NodePosY = YOffset;
		YOffset += 150;
	}

	if (GraphEditorWidget.IsValid())
	{
		GraphEditorWidget->NotifyGraphChanged();
	}

	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE




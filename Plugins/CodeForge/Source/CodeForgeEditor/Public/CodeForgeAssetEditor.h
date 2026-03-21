// CodeForgeAssetEditor.h
// FAssetEditorToolkit subclass providing the graph editor UI for UCodeForgeBlueprint.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"

class UCodeForgeBlueprint;
class UCodeForgeEdGraph;
class SGraphEditor;
class IDetailsView;

class FCodeForgeAssetEditor : public FAssetEditorToolkit
{
public:
	void InitEditor(
		const EToolkitMode::Type Mode,
		const TSharedPtr<IToolkitHost>& InitToolkitHost,
		UCodeForgeBlueprint* InBlueprint);

	// FAssetEditorToolkit interface
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;

	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;

private:
	/** The blueprint being edited. */
	UCodeForgeBlueprint* Blueprint = nullptr;

	/** The editor graph created for this blueprint. */
	UCodeForgeEdGraph* EdGraph = nullptr;

	/** The graph editor widget. */
	TSharedPtr<SGraphEditor> GraphEditorWidget;

	/** The details panel widget. */
	TSharedPtr<IDetailsView> DetailsView;

	/** Tab identifiers. */
	static const FName GraphTabId;
	static const FName DetailsTabId;

	/** Spawn the graph editor tab. */
	TSharedRef<SDockTab> SpawnGraphTab(const FSpawnTabArgs& Args);

	/** Spawn the details panel tab. */
	TSharedRef<SDockTab> SpawnDetailsTab(const FSpawnTabArgs& Args);

	/** Called when graph node selection changes. */
	void OnGraphSelectionChanged(const TSet<UObject*>& SelectedNodes);

	/** Called when the Generate toolbar button is clicked. */
	void OnGenerateClicked();

	/** Creates the editor graph if the blueprint does not already have one. */
	void CreateEdGraphIfNeeded();
};

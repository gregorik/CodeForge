#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class UCodeForgeEdGraphNode;

class SGraphNode_CodeForge : public SGraphNode
{
	SLATE_BEGIN_ARGS(SGraphNode_CodeForge) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UCodeForgeEdGraphNode* InNode);

	// SGraphNode interface
	virtual void UpdateGraphNode() override;
	virtual void CreatePinWidgets() override;
	virtual const FSlateBrush* GetShadowBrush(bool bSelected) const override;

	/** Refresh the validation overlay. Called after blueprint validation. */
	void UpdateValidationDisplay();

	// Drag-and-drop support (used in Task 11)
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override;

private:
	/** Cached pointer to the CodeForge node. */
	UCodeForgeEdGraphNode* CodeForgeNode = nullptr;

	/** Validation error overlay container. */
	TSharedPtr<SBox> ValidationBadge;
};



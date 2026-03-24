#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "CodeForgeEdGraphSchema.generated.h"

/** Schema action that spawns a new UCodeForgeEdGraphNode into the graph. */
USTRUCT()
struct FCodeForgeSchemaAction_NewNode : public FEdGraphSchemaAction
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<class UEdGraphNode> NodeTemplate = nullptr;

	FCodeForgeSchemaAction_NewNode() {}

	FCodeForgeSchemaAction_NewNode(
		FText InNodeCategory,
		FText InMenuDesc,
		FText InToolTip,
		int32 InGrouping)
		: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
	{
	}

	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
};

UCLASS()
class CODEFORGEEDITOR_API UCodeForgeEdGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()

public:
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;
};


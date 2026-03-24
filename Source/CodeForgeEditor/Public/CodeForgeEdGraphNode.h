#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "CodeForgeEdGraphNode.generated.h"

UCLASS(Abstract)
class CODEFORGEEDITOR_API UCodeForgeEdGraphNode : public UEdGraphNode
{
	GENERATED_BODY()

public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual void AllocateDefaultPins() override;
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;

	virtual void SyncToBlueprint(class UCodeForgeBlueprint* Blueprint) const {}
	virtual void SyncFromBlueprint(const class UCodeForgeBlueprint* Blueprint) {}

	/** Returns true if this node "owns" the given field name for validation error mapping. */
	virtual bool OwnsField(const FString& FieldName) const { return false; }
};


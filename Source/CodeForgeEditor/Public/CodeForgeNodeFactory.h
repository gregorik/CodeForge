#pragma once

#include "CoreMinimal.h"
#include "EdGraphUtilities.h"

class FCodeForgeNodeFactory : public FGraphPanelNodeFactory
{
public:
	virtual TSharedPtr<class SGraphNode> CreateNode(class UEdGraphNode* InNode) const override;
};



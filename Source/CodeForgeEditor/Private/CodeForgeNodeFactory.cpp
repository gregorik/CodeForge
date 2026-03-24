#include "CodeForgeNodeFactory.h"
#include "CodeForgeEdGraphNode.h"
#include "SGraphNode_CodeForge.h"

TSharedPtr<SGraphNode> FCodeForgeNodeFactory::CreateNode(UEdGraphNode* InNode) const
{
	UCodeForgeEdGraphNode* CFNode = Cast<UCodeForgeEdGraphNode>(InNode);
	if (CFNode)
	{
		TSharedPtr<SGraphNode_CodeForge> Widget = SNew(SGraphNode_CodeForge, CFNode);
		return Widget;
	}
	return nullptr;
}



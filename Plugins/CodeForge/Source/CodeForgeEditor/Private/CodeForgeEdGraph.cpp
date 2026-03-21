#include "CodeForgeEdGraph.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgeEdGraphNode.h"

void UCodeForgeEdGraph::SyncAllNodesToBlueprint()
{
	if (!CodeForgeBlueprint)
	{
		return;
	}

	CodeForgeBlueprint->Properties.Empty();
	CodeForgeBlueprint->Functions.Empty();
	CodeForgeBlueprint->RPCs.Empty();
	CodeForgeBlueprint->StructProperties.Empty();
	CodeForgeBlueprint->EnumEntries.Empty();

	for (UEdGraphNode* Node : Nodes)
	{
		UCodeForgeEdGraphNode* CFNode = Cast<UCodeForgeEdGraphNode>(Node);
		if (CFNode)
		{
			CFNode->SyncToBlueprint(CodeForgeBlueprint);
		}
	}
}

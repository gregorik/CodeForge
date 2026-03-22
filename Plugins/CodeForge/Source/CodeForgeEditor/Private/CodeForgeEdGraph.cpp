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

void UCodeForgeEdGraph::RunValidationOnNodes()
{
	if (!CodeForgeBlueprint) return;

	TArray<FCodeForgeValidationResult> Results = CodeForgeBlueprint->Validate();

	// Clear all error states
	for (UEdGraphNode* Node : Nodes)
	{
		Node->bHasCompilerMessage = false;
		Node->ErrorMsg.Empty();
		Node->ErrorType = EMessageSeverity::Info;
	}

	// Map errors to nodes by FieldName
	for (const FCodeForgeValidationResult& VR : Results)
	{
		for (UEdGraphNode* Node : Nodes)
		{
			UCodeForgeEdGraphNode* CFNode = Cast<UCodeForgeEdGraphNode>(Node);
			if (CFNode && CFNode->OwnsField(VR.FieldName))
			{
				Node->bHasCompilerMessage = true;
				if (!Node->ErrorMsg.IsEmpty())
				{
					Node->ErrorMsg += TEXT("\n");
				}
				Node->ErrorMsg += VR.Message;
				Node->ErrorType = (VR.Severity == ECodeForgeValidationSeverity::Error)
					? EMessageSeverity::Error : EMessageSeverity::Warning;
			}
		}
	}
}

void UCodeForgeEdGraph::SyncAllNodesFromBlueprint()
{
	if (!CodeForgeBlueprint)
	{
		return;
	}

	for (UEdGraphNode* Node : Nodes)
	{
		UCodeForgeEdGraphNode* CFNode = Cast<UCodeForgeEdGraphNode>(Node);
		if (CFNode)
		{
			CFNode->SyncFromBlueprint(CodeForgeBlueprint);
		}
	}
}

// Copyright (c) 2026 GregOrigin. All Rights Reserved.

#include "CodeForgeEdGraph.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgeEdGraphNode.h"
#include "CodeForgeNode_Class.h"
#include "CodeForgeNode_Struct.h"
#include "CodeForgeNode_Enum.h"
#include "CodeForgeNode_Property.h"
#include "CodeForgeNode_Function.h"

void UCodeForgeEdGraph::SyncAllNodesToBlueprint()
{
	if (!CodeForgeBlueprint)
	{
		return;
	}

	CodeForgeBlueprint->Properties.Empty();
	CodeForgeBlueprint->Functions.Empty();
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

// ---------------------------------------------------------------------------
// PopulateFromBlueprint â€” create graph nodes from blueprint data
// ---------------------------------------------------------------------------

void UCodeForgeEdGraph::PopulateFromBlueprint()
{
	if (!CodeForgeBlueprint) return;

	// Only populate if the graph is empty and the blueprint has data
	bool bHasCFNodes = false;
	for (UEdGraphNode* N : Nodes)
	{
		if (N && N->IsA<UCodeForgeEdGraphNode>())
		{
			bHasCFNodes = true;
			break;
		}
	}
	if (bHasCFNodes) return;

	const float TypeNodeX = 0.f;
	const float MemberStartX = 350.f;
	const float NodeSpacingY = 120.f;
	float NextMemberY = 0.f;

	// Helper: add a node to the graph with position and pins
	auto PlaceNode = [this](UCodeForgeEdGraphNode* Node, float PosX, float PosY)
	{
		Node->CreateNewGuid();
		Node->NodePosX = static_cast<int32>(PosX);
		Node->NodePosY = static_cast<int32>(PosY);
		this->AddNode(Node, /*bUserAction=*/false, /*bSelectNewNode=*/false);
		Node->AllocateDefaultPins();
	};

	// Helper: wire an output pin to a new node's input pin by matching category
	auto WireNodes = [](UEdGraphNode* FromNode, UEdGraphNode* ToNode)
	{
		for (UEdGraphPin* OutPin : FromNode->Pins)
		{
			if (OutPin->Direction != EGPD_Output) continue;
			for (UEdGraphPin* InPin : ToNode->Pins)
			{
				if (InPin->Direction == EGPD_Input
					&& InPin->PinType.PinCategory == OutPin->PinType.PinCategory)
				{
					OutPin->MakeLinkTo(InPin);
					return;
				}
			}
		}
	};

	// Create the type node
	UCodeForgeEdGraphNode* TypeNode = nullptr;

	switch (CodeForgeBlueprint->BlueprintKind)
	{
	case ECodeForgeBlueprintKind::Class:
	{
		UCodeForgeNode_Class* ClassNode = NewObject<UCodeForgeNode_Class>(this);
		ClassNode->ClassName = CodeForgeBlueprint->ClassName;
		ClassNode->ClassType = CodeForgeBlueprint->ClassType;
		ClassNode->bReplicated = CodeForgeBlueprint->bReplicated;
		TypeNode = ClassNode;
		break;
	}
	case ECodeForgeBlueprintKind::Struct:
	{
		UCodeForgeNode_Struct* StructNode = NewObject<UCodeForgeNode_Struct>(this);
		StructNode->ClassName = CodeForgeBlueprint->ClassName;
		TypeNode = StructNode;
		break;
	}
	case ECodeForgeBlueprintKind::Enum:
	{
		UCodeForgeNode_Enum* EnumNode = NewObject<UCodeForgeNode_Enum>(this);
		EnumNode->ClassName = CodeForgeBlueprint->ClassName;
		EnumNode->EnumEntries = CodeForgeBlueprint->EnumEntries;
		EnumNode->bBlueprintType = CodeForgeBlueprint->bBlueprintType;
		TypeNode = EnumNode;
		break;
	}
	}

	if (!TypeNode) return;
	PlaceNode(TypeNode, TypeNodeX, 0.f);

	// Properties (class or struct)
	const TArray<FCodeForgePropertyDef>& Props =
		(CodeForgeBlueprint->BlueprintKind == ECodeForgeBlueprintKind::Struct)
		? CodeForgeBlueprint->StructProperties
		: CodeForgeBlueprint->Properties;

	for (const FCodeForgePropertyDef& Prop : Props)
	{
		UCodeForgeNode_Property* PropNode = NewObject<UCodeForgeNode_Property>(this);
		PropNode->PropertyDef = Prop;
		PlaceNode(PropNode, MemberStartX, NextMemberY);
		WireNodes(TypeNode, PropNode);
		NextMemberY += NodeSpacingY;
	}

	// Functions
	for (const FCodeForgeFunctionDef& Func : CodeForgeBlueprint->Functions)
	{
		UCodeForgeNode_Function* FuncNode = NewObject<UCodeForgeNode_Function>(this);
		FuncNode->FunctionDef = Func;
		PlaceNode(FuncNode, MemberStartX, NextMemberY);
		WireNodes(TypeNode, FuncNode);
		NextMemberY += NodeSpacingY;
	}

	// Center the type node vertically relative to its members
	if (NextMemberY > 0.f)
	{
		TypeNode->NodePosY = static_cast<int32>((NextMemberY - NodeSpacingY) * 0.5f);
	}
}



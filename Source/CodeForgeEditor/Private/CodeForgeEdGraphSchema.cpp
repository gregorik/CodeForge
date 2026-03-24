#include "CodeForgeEdGraphSchema.h"
#include "CodeForgeEdGraphNode.h"
#include "ScopedTransaction.h"
#include "CodeForgeNode_Class.h"
#include "CodeForgeNode_Struct.h"
#include "CodeForgeNode_Enum.h"
#include "CodeForgeNode_Property.h"
#include "CodeForgeNode_Function.h"
#include "CodeForgeSettings.h"

// ---------------------------------------------------------------------------
// FCodeForgeSchemaAction_NewNode
// ---------------------------------------------------------------------------

UEdGraphNode* FCodeForgeSchemaAction_NewNode::PerformAction(
	UEdGraph* ParentGraph, UEdGraphPin* FromPin,
	const FVector2D Location, bool bSelectNewNode)
{
	if (!NodeTemplate || !ParentGraph)
	{
		return nullptr;
	}

	const FScopedTransaction Transaction(FText::FromString(TEXT("Add CodeForge Node")));
	ParentGraph->Modify();

	UEdGraphNode* NewNode = DuplicateObject<UEdGraphNode>(NodeTemplate, ParentGraph);
	NewNode->Modify();

	NewNode->CreateNewGuid();
	NewNode->NodePosX = static_cast<int32>(Location.X);
	NewNode->NodePosY = static_cast<int32>(Location.Y);

	ParentGraph->AddNode(NewNode, /*bUserAction=*/true, bSelectNewNode);
	NewNode->PostPlacedNewNode();
	NewNode->AllocateDefaultPins();

	// Wire the new node to the pin we dragged from
	if (FromPin)
	{
		for (UEdGraphPin* Pin : NewNode->Pins)
		{
			if (Pin->Direction != FromPin->Direction
				&& Pin->PinType.PinCategory == FromPin->PinType.PinCategory)
			{
				// Record both sides for undo
				FromPin->GetOwningNode()->Modify();
				Pin->GetOwningNode()->Modify();
				FromPin->MakeLinkTo(Pin);
				break;
			}
		}
	}

	return NewNode;
}

// ---------------------------------------------------------------------------
// CanCreateConnection
// ---------------------------------------------------------------------------

const FPinConnectionResponse UCodeForgeEdGraphSchema::CanCreateConnection(
	const UEdGraphPin* A, const UEdGraphPin* B) const
{
	if (!A || !B)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Invalid pin."));
	}

	// Ensure A is output and B is input
	const UEdGraphPin* OutputPin = A;
	const UEdGraphPin* InputPin = B;
	if (A->Direction == EGPD_Input && B->Direction == EGPD_Output)
	{
		OutputPin = B;
		InputPin = A;
	}
	else if (A->Direction == B->Direction)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Cannot connect two pins of the same direction."));
	}

	const FName& OutputCategory = OutputPin->PinType.PinCategory;
	const FName& InputCategory = InputPin->PinType.PinCategory;

	// Categories must match
	if (OutputCategory != InputCategory)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Pin types do not match."));
	}

	UEdGraphNode* OutputNode = OutputPin->GetOwningNode();
	UEdGraphNode* InputNode = InputPin->GetOwningNode();

	// Property connections: Class or Struct output -> Property input
	if (InputCategory == TEXT("PC_CodeForgeProperty"))
	{
		bool bFromClassOrStruct = OutputNode->IsA<UCodeForgeNode_Class>() || OutputNode->IsA<UCodeForgeNode_Struct>();
		bool bToProperty = InputNode->IsA<UCodeForgeNode_Property>();

		if (bFromClassOrStruct && bToProperty)
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
		}

		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Property pins must connect from a Class or Struct node."));
	}

	// Function connections: Class output -> Function input
	if (InputCategory == TEXT("PC_CodeForgeFunction"))
	{
		bool bFromClass = OutputNode->IsA<UCodeForgeNode_Class>();
		bool bToFunction = InputNode->IsA<UCodeForgeNode_Function>();

		if (bFromClass && bToFunction)
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
		}

		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Function pins must connect from a Class node."));
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Unknown pin category."));
}

// ---------------------------------------------------------------------------
// GetGraphContextActions
// ---------------------------------------------------------------------------

void UCodeForgeEdGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	{
		TSharedPtr<FCodeForgeSchemaAction_NewNode> Action = MakeShared<FCodeForgeSchemaAction_NewNode>(
			FText::FromString(TEXT("CodeForge")),
			FText::FromString(TEXT("Add Class")),
			FText::FromString(TEXT("Add a new Class definition node")),
			0);
		Action->NodeTemplate = NewObject<UCodeForgeNode_Class>(ContextMenuBuilder.OwnerOfTemporaries);
		ContextMenuBuilder.AddAction(Action);
	}

	{
		TSharedPtr<FCodeForgeSchemaAction_NewNode> Action = MakeShared<FCodeForgeSchemaAction_NewNode>(
			FText::FromString(TEXT("CodeForge")),
			FText::FromString(TEXT("Add Struct")),
			FText::FromString(TEXT("Add a new Struct definition node")),
			0);
		Action->NodeTemplate = NewObject<UCodeForgeNode_Struct>(ContextMenuBuilder.OwnerOfTemporaries);
		ContextMenuBuilder.AddAction(Action);
	}

	{
		TSharedPtr<FCodeForgeSchemaAction_NewNode> Action = MakeShared<FCodeForgeSchemaAction_NewNode>(
			FText::FromString(TEXT("CodeForge")),
			FText::FromString(TEXT("Add Enum")),
			FText::FromString(TEXT("Add a new Enum definition node")),
			0);
		Action->NodeTemplate = NewObject<UCodeForgeNode_Enum>(ContextMenuBuilder.OwnerOfTemporaries);
		ContextMenuBuilder.AddAction(Action);
	}

	{
		TSharedPtr<FCodeForgeSchemaAction_NewNode> Action = MakeShared<FCodeForgeSchemaAction_NewNode>(
			FText::FromString(TEXT("CodeForge")),
			FText::FromString(TEXT("Add Property")),
			FText::FromString(TEXT("Add a new Property node")),
			0);
		Action->NodeTemplate = NewObject<UCodeForgeNode_Property>(ContextMenuBuilder.OwnerOfTemporaries);
		ContextMenuBuilder.AddAction(Action);
	}

	{
		TSharedPtr<FCodeForgeSchemaAction_NewNode> Action = MakeShared<FCodeForgeSchemaAction_NewNode>(
			FText::FromString(TEXT("CodeForge")),
			FText::FromString(TEXT("Add Function")),
			FText::FromString(TEXT("Add a new Function node")),
			0);
		Action->NodeTemplate = NewObject<UCodeForgeNode_Function>(ContextMenuBuilder.OwnerOfTemporaries);
		ContextMenuBuilder.AddAction(Action);
	}
}

// ---------------------------------------------------------------------------
// GetPinTypeColor
// ---------------------------------------------------------------------------

FLinearColor UCodeForgeEdGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	const UCodeForgeSettings* Settings = GetDefault<UCodeForgeSettings>();

	if (PinType.PinCategory == TEXT("PC_CodeForgeProperty"))
	{
		return Settings->PropertyNodeColor;
	}

	if (PinType.PinCategory == TEXT("PC_CodeForgeFunction"))
	{
		return Settings->FunctionNodeColor;
	}

	return FLinearColor::White;
}

